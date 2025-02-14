/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "OptimalSelectorBase.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OptimalSelectorBase)
#endif

namespace hku {

string OptimalSelectorBase::str() const {
    std::ostringstream buf;
    buf << "Selector(" << name() << ", " << getParameter()
        << ", candidate systems count: " << m_pro_sys_list.size() << ")";
    return buf.str();
}

OptimalSelectorBase::OptimalSelectorBase() : SelectorBase("OptimalSelectorBase") {
    _initParams();
}

OptimalSelectorBase::OptimalSelectorBase(const string& name) : SelectorBase(name) {
    _initParams();
}

void OptimalSelectorBase::_initParams() {
    setParam<bool>("depend_on_proto_sys", true);
    setParam<string>("market", "SH");
    setParam<int>("index", 0);  // 取排序后的第 index 个结果
    setParam<int>("train_len", 100);
    setParam<int>("test_len", 20);
    setParam<bool>("parallel", false);
    setParam<bool>("trace", false);
}

void OptimalSelectorBase::_checkParam(const string& name) const {
    if ("train_len" == name) {
        HKU_ASSERT(getParam<int>("train_len") > 0);
    } else if ("test_len" == name) {
        HKU_ASSERT(getParam<int>("test_len") > 0);
    } else if ("index" == name) {
        HKU_ASSERT(getParam<int>("index") >= 0);
    } else if ("depend_on_proto_sys" == name) {
        HKU_ASSERT(getParam<bool>("depend_on_proto_sys"));
    } else if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    }
}

SystemWeightList OptimalSelectorBase::getSelected(Datetime date) {
    SystemWeightList ret;
    auto iter = m_sys_dict.find(date);
    if (iter != m_sys_dict.end()) {
        int index = getParam<int>("index");
        if (index < iter->second->size()) {
            ret.emplace_back(iter->second->at(index));
        } else {
            ret.emplace_back(iter->second->at(iter->second->size() - 1));
        }
    }
    return ret;
}

bool OptimalSelectorBase::isMatchAF(const AFPtr& af) {
    return true;
}

void OptimalSelectorBase::_reset() {
    m_sys_dict.clear();
    m_run_ranges.clear();
}

void OptimalSelectorBase::_calculate() {}

void OptimalSelectorBase::calculate(const SystemList& pf_realSysList, const KQuery& query) {
    SPEND_TIME(OptimalSelectorBase_calculate);
    HKU_IF_RETURN(m_calculated && m_query == query, void());

    m_query = query;
    m_real_sys_list = pf_realSysList;

    bool trace = getParam<bool>("trace");
    CLS_INFO_IF(trace, "candidate sys list size: {}", m_pro_sys_list.size());
    CLS_WARN_IF_RETURN(m_pro_sys_list.empty(), void(), "candidate sys list is empty!");

    // 运行时检查，而不是 addSystem 时检查，以便 WalkForwordSystem 可以直接加入未指定标的的系统列表
    for (const auto& sys : m_pro_sys_list) {
        CLS_ERROR_IF_RETURN(sys->getStock().isNull(), void(),
                            "The candidate sys ({}) was specified stock!", sys->name());
    }

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

    if (getParam<bool>("parallel")) {
        _calculate_parallel(train_ranges, dates, test_len, trace);
    } else {
        _calculate_single(train_ranges, dates, test_len, trace);
    }

    m_calculated = true;
}

void OptimalSelectorBase::_calculate_single(const vector<std::pair<size_t, size_t>>& train_ranges,
                                            const DatetimeList& dates, size_t test_len,
                                            bool trace) {
    // SPEND_TIME(OptimalSelectorBase_calculate_single);
    size_t dates_len = dates.size();
    std::shared_ptr<SystemWeightList> selected_sys_list;
    for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
        Datetime start_date = dates[train_ranges[i].first];
        Datetime end_date = dates[train_ranges[i].second];
        KQuery q = KQueryByDate(start_date, end_date, m_query.kType(), m_query.recoverType());
        CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1, total, q);

        selected_sys_list = std::make_shared<SystemWeightList>();
        for (const auto& sys : m_pro_sys_list) {
            try {
                auto nsys = sys->clone();
                nsys->run(q, true);
                double value = evaluate(nsys, end_date);
                nsys->reset();
                if (!std::isnan(value)) {
                    selected_sys_list->emplace_back(SystemWeight(nsys, value));
                }
            } catch (const std::exception& e) {
                CLS_ERROR("{}! {}", e.what(), sys->name());
            } catch (...) {
                CLS_ERROR("Unknown error! {}", sys->name());
            }
        }

        if (!selected_sys_list->empty()) {
            // 降序排列，相等时取排在候选前面的
            std::stable_sort(
              selected_sys_list->begin(), selected_sys_list->end(),
              [](const SystemWeight& a, const SystemWeight& b) { return a.weight > b.weight; });

            size_t train_start = train_ranges[i].first;
            size_t test_start = train_ranges[i].second;
            size_t test_end = test_start + test_len;
            if (test_end > dates_len) {
                test_end = dates_len;
            }

            for (size_t pos = test_start; pos < test_end; pos++) {
                m_sys_dict[dates[pos]] = selected_sys_list;
            }

            if (test_end < dates_len) {
                m_run_ranges.emplace_back(
                  RunRanges(dates[train_start], dates[test_start], dates[test_end]));
            } else {
                // K线日期只到分钟级，最后一段加1分钟
                m_run_ranges.emplace_back(RunRanges(dates[train_start], dates[test_start],
                                                    dates[test_end - 1] + Minutes(1)));
            }
        }
    }
}

void OptimalSelectorBase::_calculate_parallel(const vector<std::pair<size_t, size_t>>& train_ranges,
                                              const DatetimeList& dates, size_t test_len,
                                              bool trace) {
    // SPEND_TIME(OptimalSelectorBase_calculate_parallel);
    auto sys_list = parallel_for_index(
      0, train_ranges.size(), [this, &train_ranges, &dates, query = m_query, trace](size_t i) {
          Datetime start_date = dates[train_ranges[i].first];
          Datetime end_date = dates[train_ranges[i].second];
          KQuery q = KQueryByDate(start_date, end_date, query.kType(), query.recoverType());
          CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1, train_ranges.size(), q);

          auto selected_sys_list = std::make_shared<SystemWeightList>();
          for (const auto& sys : m_pro_sys_list) {
              try {
                  auto nsys = sys->clone();
                  nsys->run(q, true);
                  double value = evaluate(nsys, end_date);
                  nsys->reset();
                  if (!std::isnan(value)) {
                      selected_sys_list->emplace_back(SystemWeight(nsys->clone(), value));
                  }
              } catch (const std::exception& e) {
                  CLS_ERROR("{}! {}", e.what(), sys->name());
              } catch (...) {
                  CLS_ERROR("Unknown error! {}", sys->name());
              }
          }

          if (!selected_sys_list->empty()) {
              // 降序排列，相等时取排在候选前面的
              std::stable_sort(
                selected_sys_list->begin(), selected_sys_list->end(),
                [](const SystemWeight& a, const SystemWeight& b) { return a.weight > b.weight; });
          }
          return selected_sys_list;
      });

    size_t dates_len = dates.size();
    for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
        auto& selected_sys_list = sys_list[i];
        if (!selected_sys_list->empty()) {
            size_t train_start = train_ranges[i].first;
            size_t test_start = train_ranges[i].second;
            size_t test_end = test_start + test_len;
            if (test_end > dates_len) {
                test_end = dates_len;
            }

            for (size_t pos = test_start; pos < test_end; pos++) {
                m_sys_dict[dates[pos]] = selected_sys_list;
            }

            if (test_end < dates_len) {
                m_run_ranges.emplace_back(
                  RunRanges(dates[train_start], dates[test_start], dates[test_end]));
            } else {
                m_run_ranges.emplace_back(RunRanges(dates[train_start], dates[test_start],
                                                    dates[test_end - 1] + Minutes(1)));
            }
        }
    }
}

}  // namespace hku