/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "hikyuu/trade_manage/Performance.h"
#include "OptimalSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OptimalSelector)
#endif

namespace hku {

OptimalSelector::OptimalSelector() : SelectorBase("SE_Optimal") {
    setParam<bool>("depend_on_proto_sys", true);
    setParam<string>("market", "SH");
    setParam<string>("key", "帐户平均年收益率%");
    setParam<int>("mode", 0);  // 0 取最高值，1 取最低值
    setParam<int>("train_len", 100);
    setParam<int>("test_len", 20);
}

void OptimalSelector::_checkParam(const string& name) const {
    if ("mode" == name) {
        int mode = getParam<int>(name);
        HKU_ASSERT(0 == mode || 1 == mode);
    } else if ("key" == name) {
        string key = getParam<string>("key");
        HKU_CHECK(Performance::exist(key), R"(Invalid key("{}") in Performance!)", key);
    } else if ("train_len" == name) {
        HKU_ASSERT(getParam<int>("train_len") > 0);
    } else if ("test_len" == name) {
        HKU_ASSERT(getParam<int>("test_len") >= 0);
    } else if ("depend_on_proto_sys" == name) {
        HKU_ASSERT(getParam<bool>("depend_on_proto_sys"));
    } else if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    }
}

SystemWeightList OptimalSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    auto iter = m_sys_dict.find(date);
    if (iter != m_sys_dict.end()) {
        ret.emplace_back(SystemWeight(iter->second, 1.0));
    }
    return ret;
}

bool OptimalSelector::isMatchAF(const AFPtr& af) {
    return true;
}

void OptimalSelector::_reset() {
    m_sys_dict.clear();
    m_run_ranges.clear();
}

void OptimalSelector::_calculate() {}

void OptimalSelector::calculate(const SystemList& pf_realSysList, const KQuery& query) {
    HKU_IF_RETURN(m_calculated && m_query == query, void());

    m_query = query;
    m_real_sys_list = pf_realSysList;

    size_t train_len = static_cast<size_t>(getParam<int>("train_len"));
    size_t test_len = static_cast<size_t>(getParam<int>("test_len"));
    size_t data_len = train_len + test_len;

    auto dates = StockManager::instance().getTradingCalendar(query, getParam<string>("market"));
    size_t dates_len = dates.size();

    vector<std::pair<size_t, size_t>> train_ranges;
    size_t start = 0, end = train_len;
    if (end < dates_len) {
        train_ranges.emplace_back(std::make_pair(start, end));
    }
    start += test_len;
    end += test_len;
    while (end < dates_len) {
        train_ranges.emplace_back(std::make_pair(start, end));
    }

    string key = getParam<string>("key");
    int mode = getParam<int>("mode");
    Performance per;
    if (0 == mode) {
        for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
            Datetime start_date = dates[train_ranges[i].first];
            Datetime end_date = dates[train_ranges[i].second];
            KQuery q = KQueryByDate(start_date, end_date, query.kType(), query.recoverType());
            SYSPtr max_sys;
            if (m_pro_sys_list.size() == 1) {
                max_sys = m_pro_sys_list.back();
            } else {
                double max_value = std::numeric_limits<double>::min();
                for (auto& sys : m_pro_sys_list) {
                    sys->run(q);
                    per.statistics(sys->getTM(), end_date);
                    double value = per.get(key);
                    if (value > max_value) {
                        max_value = value;
                        max_sys = sys;
                    }
                }
            }

            HKU_ASSERT(max_sys);
            size_t test_start = train_ranges[i].second;
            size_t test_end = test_start + test_len;
            if (test_end > dates_len) {
                test_end = dates_len;
            }
            max_sys->reset();
            max_sys = max_sys->clone();
            for (size_t pos = test_start; pos < test_end; pos++) {
                m_sys_dict[dates[pos]] = max_sys;
            }
            m_run_ranges.emplace_back(std::make_pair(dates[test_start], dates[test_end]));
        }
    } else {
        for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
            Datetime start_date = dates[train_ranges[i].first];
            Datetime end_date = dates[train_ranges[i].second];
            KQuery q = KQueryByDate(start_date, end_date, query.kType(), query.recoverType());
            SYSPtr min_sys;
            if (m_pro_sys_list.size() == 1) {
                min_sys = m_pro_sys_list.back();
            } else {
                double min_value = std::numeric_limits<double>::max();
                for (auto& sys : m_pro_sys_list) {
                    sys->run(q);
                    per.statistics(sys->getTM(), end_date);
                    double value = per.get(key);
                    if (value > min_value) {
                        min_value = value;
                        min_sys = sys;
                    }
                }
            }

            HKU_ASSERT(min_sys);
            size_t test_start = train_ranges[i].second;
            size_t test_end = test_start + test_len;
            if (test_end > dates_len) {
                test_end = dates_len;
            }
            min_sys->reset();
            min_sys = min_sys->clone();
            for (size_t pos = test_start; pos < test_end; pos++) {
                m_sys_dict[dates[pos]] = min_sys;
            }
            m_run_ranges.emplace_back(std::make_pair(dates[test_start], dates[test_end]));
        }
    }

    m_calculated = true;
}

SEPtr HKU_API SE_Optimal() {
    return make_shared<OptimalSelector>();
}

}  // namespace hku