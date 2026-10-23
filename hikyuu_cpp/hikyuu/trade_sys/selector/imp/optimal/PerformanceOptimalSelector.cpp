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

void PerformanceOptimalSelector::_calculate_parallel(
  const vector<std::pair<size_t, size_t>>& train_ranges, const DatetimeList& dates, size_t test_len,
  bool trace) {
    // SPEND_TIME(OptimalSelector_calculate_parallel);
    string key = getParam<string>("key");
    int mode = getParam<int>("mode");
    CLS_INFO_IF(trace, "statistic key: {}, mode: {}", key, mode);

    SystemList pro_list = _cloneProtosForTraining();

    auto sys_list = global_parallel_for_index(
      0, train_ranges.size(),
      [&pro_list, &train_ranges, &dates, query = m_query, trace, key, mode](size_t i) {
          Datetime start_date = dates[train_ranges[i].first];
          Datetime end_date = dates[train_ranges[i].second];
          KQuery q = KQueryByDate(start_date, end_date, query.kType(), query.recoverType());
          CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1, train_ranges.size(), q);

          auto selected_sys_list = std::make_shared<OptimalSelectedList>();
          Performance per;
          size_t selected_index = Null<size_t>();
          SYSPtr selected_sys;
          if (pro_list.size() == 1) {
              selected_index = 0;
              selected_sys = pro_list.back()->clone();
          } else if (0 == mode) {
              double max_value = std::numeric_limits<double>::lowest();
              for (size_t j = 0, total = pro_list.size(); j < total; j++) {
                  const auto& sys = pro_list[j];
                  try {
                      // Cut off all the shared parts to avoid a parallel conflict
                      auto new_sys = sys->clone();
                      new_sys->run(q, true);
                      per.statistics(new_sys->getTM(), end_date);
                      double value = per.get(key);
                      if (std::isnan(value)) {
                          continue;
                      }
                      CLS_TRACE_IF(trace, "value: {}, stock: {}", value,
                                   sys->getStock().market_code());
                      if (value > max_value) {
                          max_value = value;
                          selected_index = j;
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
              for (size_t j = 0, total = pro_list.size(); j < total; j++) {
                  const auto& sys = pro_list[j];
                  try {
                      auto new_sys = sys->clone();
                      new_sys->run(q, true);
                      per.statistics(new_sys->getTM(), end_date);
                      double value = per.get(key);
                      if (std::isnan(value)) {
                          continue;
                      }
                      CLS_TRACE_IF(trace, "value: {}, stock: {}", value,
                                   sys->getStock().market_code());
                      if (value < min_value) {
                          min_value = value;
                          selected_index = j;
                          selected_sys = new_sys;
                      }
                  } catch (const std::exception& e) {
                      CLS_ERROR("{}! {}", e.what(), sys->name());
                  } catch (...) {
                      CLS_ERROR("Unknown error! {}", sys->name());
                  }
              }
          }

          if (selected_sys) {
              selected_sys_list->emplace_back(
                OptimalSelectedItem{selected_index, 1.0, selected_sys});
          }
          return selected_sys_list;
      });

    _applySelected(sys_list, dates, train_ranges, test_len, trace);
}

SEPtr HKU_API SE_PerformanceOptimal(const string& key, int mode) {
    PerformanceOptimalSelector* p = new PerformanceOptimalSelector();
    p->setParam<string>("key", key);
    p->setParam<int>("mode", mode);
    return SEPtr(p);
}

}  // namespace hku