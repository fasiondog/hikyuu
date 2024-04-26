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
    setParam<int>("field_ix", 0);
    setParam<string>("field_name", "");

    // 某些信息如每股收益，只使用年报计算
    setParam<bool>("only_year_report", false);

    // 某些信息如每股收益，需要动态计算，如全年收益在只有一季报时，使用一季报*4进行预估
    setParam<bool>("dynamic", false);
}

IFinance::IFinance(const KData& k) : IndicatorImp("FINANCE", 1) {
    setParam<int>("field_ix", 0);
    setParam<string>("field_name", "");
    setParam<KData>("kdata", k);
    setParam<bool>("only_year_report", false);
    setParam<bool>("dynamic", false);
    IFinance::_calculate(Indicator());
}

void IFinance::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData kdata = getContext();
    size_t total = kdata.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);

    Stock stock = kdata.getStock();
    auto finances = stock.getHistoryFinance();

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
    auto dates = kdata.getDatetimeList();
    auto* k = kdata.data();

    size_t finances_total = finances.size();
    size_t cur_kix = 0;
    size_t pos = 0;
    while (pos < finances_total && cur_kix < total) {
        auto value = finances[pos].values.at(field_ix);
        if (pos + 1 == finances_total) {
            while (cur_kix < total && finances[pos].reportDate <= k[cur_kix].datetime) {
                if (dynamic) {
                    long month = finances[pos].fileDate.month();
                    if (3L == month) {
                        // 一季报
                        dst[cur_kix] = value * 4;
                    } else if (6L == month) {
                        // 半年报
                        dst[cur_kix] = value * 2;
                    } else if (9L == month) {
                        // 三季报
                        dst[cur_kix] = value / 3.0 * 4.0;
                    } else {
                        // 年报
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
                        // 一季报
                        dst[cur_kix] = value * 4;
                    } else if (6L == month) {
                        // 半年报
                        dst[cur_kix] = value * 2;
                    } else if (9L == month) {
                        // 三季报
                        dst[cur_kix] = value / 3.0 * 4.0;
                    } else {
                        // 年报
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
    auto p = make_shared<IFinance>(k);
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
    auto p = make_shared<IFinance>(k);
    p->setParam<int>("field_ix", -1);
    p->setParam<string>("field_name", field_name);
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hku