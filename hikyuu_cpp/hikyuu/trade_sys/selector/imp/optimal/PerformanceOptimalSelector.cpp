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
    setParam<string>("key", "Account Avg Annual Return %");
    setParam<int>("mode", 0);  // 0 takes the highest value, 1 takes the lowest value
}

void PerformanceOptimalSelector::_checkParam(const string& name) const {
    OptimalSelectorBase::_checkParam(name);
    if ("mode" == name) {
        int mode = getParam<int>(name);
        HKU_ASSERT(0 == mode || 1 == mode);
    }
}

SelectorPtr PerformanceOptimalSelector::_clone() {
    return std::make_shared<PerformanceOptimalSelector>();
}

void PerformanceOptimalSelector::calculate(const SystemList& pf_realSysList, const KQuery& query) {
    // SPEND_TIME(OptimalSelector_calculate);
    HKU_IF_RETURN(m_calculated && m_query == query, void());

    m_query = query;
    m_real_sys_list = pf_realSysList;
    // Drop the previous selection first: a second calculate (a new query, a portfolio re-run) must
    // not append duplicate entries
    m_sys_dict.clear();
    m_run_ranges.clear();

    DatetimeList dates;
    vector<std::pair<size_t, size_t>> train_ranges;
    HKU_IF_RETURN(!_prepareCalculate(query, dates, train_ranges), void());

    string key = getParam<string>("key");
    int mode = getParam<int>("mode");
    bool trace = getParam<bool>("trace");
    CLS_INFO_IF(trace, "statistic key: {}, mode: {}", key, mode);

    _calculate_parallel(train_ranges, dates, key, mode,
                        static_cast<size_t>(getParam<int>("test_len")), trace);

    m_calculated = true;
}

void PerformanceOptimalSelector::_calculate_parallel(
  const vector<std::pair<size_t, size_t>>& train_ranges, const DatetimeList& dates,
  const string& key, int mode, size_t test_len, bool trace) {
    // SPEND_TIME(OptimalSelector_calculate_parallel);
    // The training runs the candidate clones concurrently and each range has its own query, so the
    // candidates must not share their EV (shared_ev defaults to true), otherwise the racing
    // setQuery calls with different queries would cross-clobber the EV state. The local
    // protos take effect from the next clone level on, leaving the user prototypes untouched.
    SystemList pro_list;
    pro_list.reserve(m_pro_sys_list.size());
    for (const auto& sys : m_pro_sys_list) {
        auto local = sys->clone();
        local->setParam<bool>("shared_ev", false);
        pro_list.emplace_back(local);
    }

    auto sys_list = global_parallel_for_index(
      0, train_ranges.size(),
      [&pro_list, &train_ranges, &dates, query = m_query, trace, key, mode](size_t i) {
          Datetime start_date = dates[train_ranges[i].first];
          Datetime end_date = dates[train_ranges[i].second];
          KQuery q = KQueryByDate(start_date, end_date, query.kType(), query.recoverType());
          CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1, train_ranges.size(), q);

          Performance per;
          SYSPtr selected_sys;
          if (pro_list.size() == 1) {
              selected_sys = pro_list.back()->clone();
          } else if (0 == mode) {
              double max_value = std::numeric_limits<double>::lowest();
              for (const auto& sys : pro_list) {
                  try {
                      // Cut off all the shared parts to avoid a parallel conflict
                      auto new_sys = sys->clone();
                      new_sys->run(q, true);
                      per.statistics(new_sys->getTM(), end_date);
                      double value = per.get(key);
                      if (std::isnan(value)) {
                          continue;
                      }
                      CLS_TRACE_IF(trace, "value: {}, sys: {}", value, new_sys->name());
                      if (value > max_value) {
                          max_value = value;
                          selected_sys = new_sys;
                      }
                  } catch (const std::exception& e) {
                      CLS_ERROR("{}! {}", e.what(), sys->name());
                  } catch (...) {
                      CLS_ERROR("Unknown error! {}", sys->name());
                  }
              }
          } else if (1 == mode) {
              double min_value = std::numeric_limits<double>::max();
              for (const auto& sys : pro_list) {
                  try {
                      auto new_sys = sys->clone();
                      new_sys->run(q, true);
                      per.statistics(new_sys->getTM(), end_date);
                      double value = per.get(key);
                      if (std::isnan(value)) {
                          continue;
                      }
                      CLS_TRACE_IF(trace, "value: {}, sys: {}", value, sys->name());
                      if (value < min_value) {
                          min_value = value;
                          selected_sys = new_sys;
                      }
                  } catch (const std::exception& e) {
                      CLS_ERROR("{}! {}", e.what(), sys->name());
                  } catch (...) {
                      CLS_ERROR("Unknown error! {}", sys->name());
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
                m_sys_dict[dates[pos]] =
                  std::make_shared<SystemWeightList>(1, SystemWeight(selected_sys, 1.0));
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