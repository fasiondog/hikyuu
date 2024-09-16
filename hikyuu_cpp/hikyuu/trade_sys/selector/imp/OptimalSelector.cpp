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
    setParam<bool>("trace", false);
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
        HKU_ASSERT(getParam<int>("test_len") > 0);
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

void OptimalSelector::_addSystem(const SYSPtr& sys) {
    CLS_CHECK(!sys->getStock().isNull(), "Invalid system({}) for unspecified stock!", sys->name());
}

void OptimalSelector::_calculate() {}

void OptimalSelector::calculate(const SystemList& pf_realSysList, const KQuery& query) {
    HKU_IF_RETURN(m_calculated && m_query == query, void());

    m_query = query;
    m_real_sys_list = pf_realSysList;

    bool trace = getParam<bool>("trace");
    CLS_INFO_IF(trace, "candidate sys list size: {}", m_pro_sys_list.size());
    // for (const auto& sys : m_pro_sys_list) {
    //     HKU_DEBUG_IF(trace, "[SE_Optimal] {}", sys->name());
    // }

    size_t train_len = static_cast<size_t>(getParam<int>("train_len"));
    size_t test_len = static_cast<size_t>(getParam<int>("test_len"));

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
        start += test_len;
        end += test_len;
    }

    string key = getParam<string>("key");
    int mode = getParam<int>("mode");
    CLS_INFO_IF(trace, "statistic key: {}, mode: {}", key, mode);

    Performance per;
    for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
        Datetime start_date = dates[train_ranges[i].first];
        Datetime end_date = dates[train_ranges[i].second];
        KQuery q = KQueryByDate(start_date, end_date, query.kType(), query.recoverType());
        CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1, total, q);
        SYSPtr selected_sys;
        if (m_pro_sys_list.size() == 1) {
            selected_sys = m_pro_sys_list.back();
        } else if (0 == mode) {
            double max_value = std::numeric_limits<double>::lowest();
            for (const auto& sys : m_pro_sys_list) {
                sys->run(q, true);
                per.statistics(sys->getTM(), end_date);
                double value = per.get(key);
                CLS_TRACE_IF(trace, "value: {}, sys: {}", value, sys->name());
                if (value > max_value) {
                    max_value = value;
                    selected_sys = sys;
                }
            }
        } else if (1 == mode) {
            double min_value = std::numeric_limits<double>::max();
            for (const auto& sys : m_pro_sys_list) {
                sys->run(q, true);
                per.statistics(sys->getTM(), end_date);
                double value = per.get(key);
                CLS_TRACE_IF(trace, "value: {}, sys: {}", value, sys->name());
                if (value < min_value) {
                    min_value = value;
                    selected_sys = sys;
                }
            }
        }

        HKU_ASSERT(selected_sys);
        size_t test_start = train_ranges[i].second;
        size_t test_end = test_start + test_len;
        if (test_end > dates_len) {
            test_end = dates_len;
        }
        selected_sys->reset();
        selected_sys = selected_sys->clone();
        for (size_t pos = test_start; pos < test_end; pos++) {
            m_sys_dict[dates[pos]] = selected_sys;
        }
        if (test_end < dates_len) {
            m_run_ranges.emplace_back(std::make_pair(dates[test_start], dates[test_end]));
        } else {
            m_run_ranges.emplace_back(
              std::make_pair(dates[test_start], dates[test_end - 1] + Seconds(1)));
        }
        CLS_INFO_IF(trace, "iteration: {}, selected_sys: {}", i + 1, selected_sys->name());
    }

    m_calculated = true;
}

SEPtr HKU_API SE_Optimal() {
    return make_shared<OptimalSelector>();
}

}  // namespace hku