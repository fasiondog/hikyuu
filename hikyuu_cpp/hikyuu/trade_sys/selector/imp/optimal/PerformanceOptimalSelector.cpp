/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "hikyuu/trade_manage/Performance.h"
#include "PerformanceOptimalSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::PerformanceOptimalSelector)
#endif

namespace hku {

PerformanceOptimalSelector::PerformanceOptimalSelector()
: OptimalSelectorBase("SE_PerformanceOptimal") {
    setParam<string>("key", "帐户平均年收益率%");
    setParam<int>("mode", 0);  // 0 取最高值，1 取最低值
}

void PerformanceOptimalSelector::_checkParam(const string& name) const {
    OptimalSelectorBase::_checkParam(name);
    if ("mode" == name) {
        int mode = getParam<int>(name);
        HKU_ASSERT(0 == mode || 1 == mode);
    } else if ("key" == name) {
        string key = getParam<string>("key");
        HKU_CHECK(Performance::exist(key), R"(Invalid key("{}") in Performance!)", key);
    }
}

SystemWeightList PerformanceOptimalSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    auto iter = this->m_sys_dict.find(date);
    if (iter != this->m_sys_dict.end()) {
        ret.emplace_back(SystemWeight(iter->second, 1.0));
    }
    return ret;
}

SelectorPtr PerformanceOptimalSelector::_clone() {
    return std::make_shared<PerformanceOptimalSelector>();
}

void PerformanceOptimalSelector::_reset() {
    OptimalSelectorBase::_reset();
    m_sys_dict.clear();
}

void PerformanceOptimalSelector::calculate(const SystemList& pf_realSysList, const KQuery& query) {
    SPEND_TIME(OptimalSelector_calculate);
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

    string key = getParam<string>("key");
    int mode = getParam<int>("mode");
    CLS_INFO_IF(trace, "statistic key: {}, mode: {}", getParam<string>("key"),
                getParam<int>("mode"));

    if (getParam<bool>("parallel")) {
        _calculate_parallel(train_ranges, dates, key, mode, test_len, trace);
    } else {
        _calculate_single(train_ranges, dates, key, mode, test_len, trace);
    }

    m_calculated = true;
}

void PerformanceOptimalSelector::_calculate_single(
  const vector<std::pair<size_t, size_t>>& train_ranges, const DatetimeList& dates,
  const string& key, int mode, size_t test_len, bool trace) {
    // SPEND_TIME(OptimalSelector_calculate_single);
    size_t dates_len = dates.size();
    Performance per;
    for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
        Datetime start_date = dates[train_ranges[i].first];
        Datetime end_date = dates[train_ranges[i].second];
        KQuery q = KQueryByDate(start_date, end_date, m_query.kType(), m_query.recoverType());
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

        if (selected_sys) {
            selected_sys->reset();
            selected_sys = selected_sys->clone();

            size_t train_start = train_ranges[i].first;
            size_t test_start = train_ranges[i].second;
            size_t test_end = test_start + test_len;
            if (test_end > dates_len) {
                test_end = dates_len;
            }

            for (size_t pos = test_start; pos < test_end; pos++) {
                m_sys_dict[dates[pos]] = selected_sys;
            }

            if (test_end < dates_len) {
                m_run_ranges.emplace_back(
                  RunRanges(dates[train_start], dates[test_start], dates[test_end]));
            } else {
                // K线日期只到分钟级，最后一段加1分钟
                m_run_ranges.emplace_back(RunRanges(dates[train_start], dates[test_start],
                                                    dates[test_end - 1] + Minutes(1)));
            }

            CLS_INFO_IF(trace, "iteration: {}, selected_sys: {}", i + 1, selected_sys->name());
        }
    }
}

void PerformanceOptimalSelector::_calculate_parallel(
  const vector<std::pair<size_t, size_t>>& train_ranges, const DatetimeList& dates,
  const string& key, int mode, size_t test_len, bool trace) {
    // SPEND_TIME(OptimalSelector_calculate_parallel);
    auto sys_list = parallel_for_index(
      0, train_ranges.size(),
      [this, &train_ranges, &dates, query = m_query, trace, key, mode](size_t i) {
          Datetime start_date = dates[train_ranges[i].first];
          Datetime end_date = dates[train_ranges[i].second];
          KQuery q = KQueryByDate(start_date, end_date, query.kType(), query.recoverType());
          CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1, train_ranges.size(), q);

          Performance per;
          SYSPtr selected_sys;
          if (m_pro_sys_list.size() == 1) {
              selected_sys = m_pro_sys_list.back()->clone();
          } else if (0 == mode) {
              double max_value = std::numeric_limits<double>::lowest();
              for (const auto& sys : m_pro_sys_list) {
                  // 切断所有共享组件，避免并行冲突
                  auto new_sys = sys->clone();
                  new_sys->run(q, true);
                  per.statistics(new_sys->getTM(), end_date);
                  double value = per.get(key);
                  CLS_TRACE_IF(trace, "value: {}, sys: {}", value, new_sys->name());
                  if (value > max_value) {
                      max_value = value;
                      selected_sys = new_sys;
                  }
              }
          } else if (1 == mode) {
              double min_value = std::numeric_limits<double>::max();
              for (const auto& sys : m_pro_sys_list) {
                  auto new_sys = sys->clone();
                  new_sys->run(q, true);
                  per.statistics(new_sys->getTM(), end_date);
                  double value = per.get(key);
                  CLS_TRACE_IF(trace, "value: {}, sys: {}", value, sys->name());
                  if (value < min_value) {
                      min_value = value;
                      selected_sys = new_sys;
                  }
              }
          }

          return selected_sys;
      });

    size_t dates_len = dates.size();
    for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
        auto& selected_sys = sys_list[i];
        if (selected_sys) {
            selected_sys->reset();

            size_t train_start = train_ranges[i].first;
            size_t test_start = train_ranges[i].second;
            size_t test_end = test_start + test_len;
            if (test_end > dates_len) {
                test_end = dates_len;
            }

            for (size_t pos = test_start; pos < test_end; pos++) {
                m_sys_dict[dates[pos]] = selected_sys;
            }

            if (test_end < dates_len) {
                m_run_ranges.emplace_back(
                  RunRanges(dates[train_start], dates[test_start], dates[test_end]));
            } else {
                m_run_ranges.emplace_back(RunRanges(dates[train_start], dates[test_start],
                                                    dates[test_end - 1] + Minutes(1)));
            }

            CLS_INFO_IF(trace, "iteration: {}, selected_sys: {}", i + 1, selected_sys->name());
        }
    }
}

SEPtr HKU_API SE_PerformanceOptimal(const string& key, int mode) {
    PerformanceOptimalSelector* p = new PerformanceOptimalSelector();
    p->setParam<string>("key", key);
    p->setParam<int>("mode", mode);
    return SEPtr(p);
}

}  // namespace hku