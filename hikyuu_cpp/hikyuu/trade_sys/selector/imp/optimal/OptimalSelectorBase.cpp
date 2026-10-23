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
    setParam<int>("index", 0);  // Take the index-th result after the sorting
    setParam<int>("train_len", 100);
    setParam<int>("test_len", 20);
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

SystemWeightList OptimalSelectorBase::_getSelected(Datetime date) {
    SystemWeightList ret;
    auto iter = m_sys_dict.find(date);
    if (iter == m_sys_dict.end() || iter->second->empty()) {
        return ret;
    }

    const auto& items = *(iter->second);
    size_t index = static_cast<size_t>(getParam<int>("index"));
    const auto& item = index < items.size() ? items[index] : items.back();
    auto sys = _selectedSystem(item.index, item.trained);
    if (sys) {
        ret.emplace_back(SystemWeight(sys, item.weight));
    }
    return ret;
}

SYSPtr OptimalSelectorBase::_selectedSystem(size_t candIndex, const SYSPtr& trained) const {
    // The aggregates bind every running system to its prototype: that mapping is the only reliable
    // identity source. The positional fallback keeps the same-order contract for the callers that
    // pass a real list without binding
    if (candIndex < m_pro_sys_list.size()) {
        auto iter = m_proto_to_real.find(m_pro_sys_list[candIndex].get());
        if (iter != m_proto_to_real.end()) {
            return iter->second;
        }
    }
    if (m_real_sys_list.size() == m_pro_sys_list.size() && candIndex < m_real_sys_list.size()) {
        return m_real_sys_list[candIndex];
    }
    return trained;
}

void OptimalSelectorBase::bindRealToProto(const SYSPtr& real, const SYSPtr& proto) {
    if (real && proto) {
        m_proto_to_real[proto.get()] = real;
    }
}

void OptimalSelectorBase::_reset() {
    m_sys_dict.clear();
    m_run_ranges.clear();
}

void OptimalSelectorBase::_calculate() {}

bool OptimalSelectorBase::_prepareCalculate(const KQuery& query, DatetimeList& dates,
                                            vector<std::pair<size_t, size_t>>& train_ranges) const {
    bool trace = getParam<bool>("trace");
    CLS_INFO_IF(trace, "candidate sys list size: {}", m_pro_sys_list.size());
    if (m_pro_sys_list.empty()) {
        CLS_WARN("candidate sys list is empty, the selection will be empty!");
        return false;
    }

    // The check is done at runtime rather than in addSystem, so that WalkForwardSystem can add the
    // system list without a given security directly
    for (const auto& sys : m_pro_sys_list) {
        if (sys->getStock().isNull()) {
            CLS_ERROR("The candidate sys ({}) has no stock specified, the selection will be empty!",
                      sys->name());
            return false;
        }
    }

    size_t train_len = static_cast<size_t>(getParam<int>("train_len"));
    size_t test_len = static_cast<size_t>(getParam<int>("test_len"));

    dates = StockManager::instance().getTradingCalendar(query, getParam<string>("market"));
    size_t dates_len = dates.size();

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
    if (train_ranges.empty()) {
        CLS_WARN(
          "No available train range, nothing will be selected! train_len: {}, test_len: {}, "
          "dates size: {}",
          train_len, test_len, dates_len);
    }
    return true;
}

void OptimalSelectorBase::calculate(const SystemList& pf_realSysList, const KQuery& query) {
    SPEND_TIME(OptimalSelectorBase_calculate);
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

    _calculate_parallel(train_ranges, dates, static_cast<size_t>(getParam<int>("test_len")),
                        getParam<bool>("trace"));

    m_calculated = true;
}

void OptimalSelectorBase::_calculate_parallel(const vector<std::pair<size_t, size_t>>& train_ranges,
                                              const DatetimeList& dates, size_t test_len,
                                              bool trace) {
    // SPEND_TIME(OptimalSelectorBase_calculate_parallel);
    SystemList pro_list = _cloneProtosForTraining();

    auto sys_list = global_parallel_for_index(
      0, train_ranges.size(),
      [this, &pro_list, &train_ranges, &dates, query = m_query, trace](size_t i) {
          Datetime start_date = dates[train_ranges[i].first];
          Datetime end_date = dates[train_ranges[i].second];
          KQuery q = KQueryByDate(start_date, end_date, query.kType(), query.recoverType());
          CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1, train_ranges.size(), q);

          auto selected_sys_list = std::make_shared<OptimalSelectedList>();
          for (size_t j = 0, total = pro_list.size(); j < total; j++) {
              const auto& sys = pro_list[j];
              try {
                  auto nsys = sys->clone();
                  nsys->run(q, true);
                  double value = evaluate(nsys, end_date);
                  nsys->reset();
                  if (!std::isnan(value)) {
                      selected_sys_list->emplace_back(OptimalSelectedItem{j, value, nsys});
                  }
              } catch (const std::exception& e) {
                  CLS_ERROR("{}! {}", e.what(), sys->name());
              } catch (...) {
                  CLS_ERROR("Unknown error! {}", sys->name());
              }
          }

          if (!selected_sys_list->empty()) {
              // Sort in the descending order; on equality the one earlier in the candidates is
              // taken
              std::stable_sort(selected_sys_list->begin(), selected_sys_list->end(),
                               [](const OptimalSelectedItem& a, const OptimalSelectedItem& b) {
                                   return a.weight > b.weight;
                               });
          }
          return selected_sys_list;
      });

    _applySelected(sys_list, dates, train_ranges, test_len, trace);
}

SystemList OptimalSelectorBase::_cloneProtosForTraining() const {
    SystemList pro_list;
    pro_list.reserve(m_pro_sys_list.size());
    for (const auto& sys : m_pro_sys_list) {
        auto local = sys->clone();
        local->setParam<bool>("shared_ev", false);
        // Mode C shares the parent account, the training copies must not book there
        local->setParam<bool>("shared_tm", false);
        pro_list.emplace_back(local);
    }

    if (!m_real_sys_list.empty() && m_real_sys_list.size() != m_pro_sys_list.size()) {
        CLS_WARN(
          "The real system list size ({}) differs from the prototype list size ({}), the selected "
          "systems cannot be mapped back to the running list!",
          m_real_sys_list.size(), m_pro_sys_list.size());
    }
    return pro_list;
}

void OptimalSelectorBase::_applySelected(
  const vector<std::shared_ptr<OptimalSelectedList>>& sys_list, const DatetimeList& dates,
  const vector<std::pair<size_t, size_t>>& train_ranges, size_t test_len, bool trace) {
    size_t dates_len = dates.size();
    for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
        const auto& selected_sys_list = sys_list[i];
        if (selected_sys_list->empty()) {
            continue;
        }
        // Run directly when no real list is given
        for (auto& item : *selected_sys_list) {
            if (item.trained) {
                item.trained->reset();
            }
        }

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
            m_run_ranges.emplace_back(
              RunRanges(dates[train_start], dates[test_start], dates[test_end - 1] + Minutes(1)));
        }

        CLS_INFO_IF(trace, "iteration: {}, selected_sys: {}", i + 1,
                    selected_sys_list->front().trained
                      ? selected_sys_list->front().trained->getStock().market_code()
                      : std::string());
    }
}

}  // namespace hku