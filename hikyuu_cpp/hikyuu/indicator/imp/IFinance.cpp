/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#include "hikyuu/StockManager.h"
#include "IFinance.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IFinance)
#endif

namespace hku {

IFinance::IFinance() : IndicatorImp("FINANCE", 1) {
    m_need_context = true;
    setParam<int>("field_ix", 0);
    setParam<string>("field_name", "");

    // Some information such as the earnings per share is calculated with the annual report only
    setParam<bool>("only_year_report", false);

    // Some information such as the earnings per share needs a dynamic calculation; for example,
    // when only the Q1 report exists, the annual earnings is estimated with the Q1 report * 4
    setParam<bool>("dynamic", false);
}

void IFinance::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& kdata = getContext();
    size_t total = kdata.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);
    _increment_calculate(data, 0);
}

void IFinance::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& kdata = getContext();
    size_t total = kdata.size();
    auto finances = kdata.getStock().getHistoryFinance();

    if (getParam<bool>("only_year_report")) {
        vector<HistoryFinanceInfo> tmp_finances;
        for (auto&& finance : finances) {
            if (finance.fileDate.month() == 12L) {
                tmp_finances.emplace_back(std::move(finance));
            }
        }
        finances = std::move(tmp_finances);
    }

    if (finances.empty()) {
        m_discard = total;
        return;
    }

    int field_ix = getParam<int>("field_ix");
    string field_name = getParam<string>("field_name");
    if (field_ix < 0 && !field_name.empty()) {
        field_ix = static_cast<int>(
          StockManager::instance().getHistoryFinanceFieldIndex(getParam<string>("field_name")));
    }

    bool dynamic = getParam<bool>("dynamic");
    auto* dst = this->data();
    const auto* k = kdata.data();

    size_t finances_total = finances.size();

    // Fixed #25 (gitee): the quarterly report and the annual report may conflict when published on
    // the same day the problem that FINANCE(kdata, 231) and FINANCE(kdata, 95) have no annual
    // report data (231 is the fourth quarter)
    for (size_t i = finances_total - 1; i > 0; --i) {
        if (finances[i - 1].reportDate >= finances[i].reportDate) {
            finances[i - 1].reportDate = finances[i].reportDate - TimeDelta(1);
        }
    }

    size_t cur_kix = start_pos;
    size_t pos = 0;
    while (pos < finances_total && cur_kix < total) {
        auto value = finances[pos].values.at(field_ix);
        if (pos + 1 == finances_total) {
            while (cur_kix < total && finances[pos].reportDate <= k[cur_kix].datetime) {
                if (dynamic) {
                    long month = finances[pos].fileDate.month();
                    if (3L == month) {
                        // Q1 report
                        dst[cur_kix] = value * 4;
                    } else if (6L == month) {
                        // Half-year report
                        dst[cur_kix] = value * 2;
                    } else if (9L == month) {
                        // Q3 report
                        dst[cur_kix] = value / 3.0 * 4.0;
                    } else {
                        // Annual report
                        dst[cur_kix] = value;
                    }
                } else {
                    dst[cur_kix] = value;
                }
                cur_kix++;
            }
        } else {
            while (cur_kix < total && finances[pos].reportDate <= k[cur_kix].datetime &&
                   finances[pos + 1].reportDate > k[cur_kix].datetime) {
                if (dynamic) {
                    long month = finances[pos].fileDate.month();
                    if (3L == month) {
                        // Q1 report
                        dst[cur_kix] = value * 4;
                    } else if (6L == month) {
                        // Half-year report
                        dst[cur_kix] = value * 2;
                    } else if (9L == month) {
                        // Q3 report
                        dst[cur_kix] = value / 3.0 * 4.0;
                    } else {
                        // Annual report
                        dst[cur_kix] = value;
                    }
                } else {
                    dst[cur_kix] = value;
                }
                cur_kix++;
            }
        }
        pos++;
    }
}

Indicator HKU_API FINANCE(int field_ix) {
    auto p = make_shared<IFinance>();
    p->setParam<int>("field_ix", field_ix);
    return Indicator(p);
}

Indicator HKU_API FINANCE(const KData& k, int field_ix) {
    auto p = make_shared<IFinance>();
    p->setParam<int>("field_ix", field_ix);
    p->setContext(k);
    return Indicator(p);
}

Indicator HKU_API FINANCE(const string& field_name) {
    auto p = make_shared<IFinance>();
    p->setParam<int>("field_ix", -1);
    p->setParam<string>("field_name", field_name);
    return Indicator(p);
}

Indicator HKU_API FINANCE(const KData& k, const string& field_name) {
    auto p = make_shared<IFinance>();
    p->setParam<int>("field_ix", -1);
    p->setParam<string>("field_name", field_name);
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hku