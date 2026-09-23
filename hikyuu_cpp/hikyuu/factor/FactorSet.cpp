/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <algorithm>

#include "hikyuu/StockManager.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/plugin/factor.h"
#include "hikyuu/plugin/device.h"
#include "FactorSet.h"
#include "imp/CompiledFactorPlan.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const FactorSet& set) {
    os << set.str();
    return os;
}

string FactorSet::str() const {
    return fmt::format("FactorSet({}, {}, {}, {})", name(), ktype(), size(), block());
}

FactorSet::FactorSet() : m_data(make_shared<Data>()) {}

FactorSet::FactorSet(const IndicatorList& inds, const KQuery::KType& ktype)
: m_data(make_shared<Data>()) {
    m_data->name = fmt::format("FSET_{}", Datetime::now().ticks());
    m_data->ktype = ktype;
    for (const auto& factor : inds) {
        add(factor);
    }
}

FactorSet::FactorSet(const std::unordered_map<string, Indicator>& inds, const KQuery::KType& ktype)
: m_data(make_shared<Data>()) {
    m_data->name = fmt::format("FSET_{}", Datetime::now().ticks());
    m_data->ktype = ktype;
    for (const auto& item : inds) {
        add(item.first, item.second);
    }
}

FactorSet::FactorSet(const string& name, const KQuery::KType& ktype, const Block& block)
: m_data(make_shared<Data>()) {
    m_data->name = utf8_to_upper(name);
    m_data->ktype = ktype;
    m_data->block = block;
}

FactorSet::FactorSet(const FactorList& factors, const KQuery::KType& ktype, const Block& block,
                     const string& name)
: m_data(make_shared<Data>()) {
    m_data->name = utf8_to_upper(name);
    m_data->ktype = ktype;
    m_data->block = block;
    add(factors);
}

FactorSet::FactorSet(const FactorSet& other) : m_data(other.m_data) {}

FactorSet::FactorSet(FactorSet&& other) : m_data(std::move(other.m_data)) {}

FactorSet& FactorSet::operator=(const FactorSet& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data = other.m_data;
    return *this;
}

FactorSet& FactorSet::operator=(FactorSet&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data = std::move(other.m_data);
    return *this;
}

void FactorSet::add(const Factor& factor) {
    HKU_CHECK(!factor.isNull(), "Factor is null!");
    HKU_CHECK(factor.ktype() == m_data->ktype, "ktype not match!");
    HKU_CHECK(factor.block() == m_data->block, "block not match!");

    const string& factor_name = factor.name();

    // Check whether a factor with the same name exists already
    auto it = m_data->nameIndexMap.find(factor_name);
    if (it != m_data->nameIndexMap.end()) {
        // A factor with the same name exists, overwrite it
        size_t index = it->second;
        m_data->factors[index] = factor;
        HKU_WARN("Factor '{}' already exists, it will be overwritten!", factor_name);

    } else {
        // Append the new factor to the end of the vector
        size_t index = m_data->factors.size();
        m_data->factors.push_back(factor);
        // Record the name to index mapping in the map
        m_data->nameIndexMap[factor_name] = index;
    }
}

void FactorSet::add(const string& name, const Indicator& ind) {
    add(Factor(name, ind, m_data->ktype, "", "", false, Datetime::min(), m_data->block));
}

void FactorSet::add(const Indicator& ind) {
    auto it = m_data->nameIndexMap.find(ind.name());
    if (it != m_data->nameIndexMap.end()) {
        add(fmt::format("{}_{}", ind.name(), Datetime::now().ticks()), ind);
    } else {
        add(ind.name(), ind);
    }
}

void FactorSet::add(const FactorList& factors) {
    for (const auto& factor : factors) {
        add(factor);
    }
}

void FactorSet::add(const IndicatorList& inds) {
    for (const auto& ind : inds) {
        add(ind);
    }
}

void FactorSet::add(const std::map<string, Indicator>& inds) {
    for (const auto& ind : inds) {
        add(ind.first, ind.second);
    }
}

void FactorSet::remove(const string& name) {
    auto it = m_data->nameIndexMap.find(name);
    if (it == m_data->nameIndexMap.end()) {
        return;  // The factor does not exist
    }

    size_t index_to_remove = it->second;
    size_t last_index = m_data->factors.size() - 1;

    // When the element to delete is not the last one, the index of the following element must be
    // adjusted
    if (index_to_remove != last_index) {
        // Move the last element to the position to be deleted
        m_data->factors[index_to_remove] = std::move(m_data->factors[last_index]);
        // Update the index of the moved element in the map
        const string& moved_factor_name = m_data->factors[index_to_remove].name();
        m_data->nameIndexMap[moved_factor_name] = index_to_remove;
    }

    // Delete the last element and the map entry
    m_data->factors.pop_back();
    m_data->nameIndexMap.erase(it);
}

bool FactorSet::have(const string& name) const noexcept {
    return m_data->nameIndexMap.find(name) != m_data->nameIndexMap.end();
}

const Factor& FactorSet::get(const string& name) const {
    auto it = m_data->nameIndexMap.find(name);
    HKU_CHECK(it != m_data->nameIndexMap.end(), "Factor '{}' not found!", name);
    return m_data->factors[it->second];
}

void FactorSet::save_to_db() const {
    saveFactorSet(*this);
}

void FactorSet::remove_from_db() const {
    removeFactorSet(name(), ktype());
}

void FactorSet::load_from_db() {
    FactorSet loaded_set = getFactorSet(name(), ktype());
    // The object returned by getFactorSet is Null, which is global
    if (!loaded_set.isNull()) {
        m_data = std::move(loaded_set.m_data);
    }
}

vector<IndicatorList> FactorSet::getValues(const StockList& stocks, const KQuery& query, bool align,
                                           bool fill_null, bool tovalue, bool check,
                                           const DatetimeList& align_dates) const {
    // SPEND_TIME(FactorSet_getValues);
    if (check) {
        if (!block().empty()) {
            for (auto& stock : stocks) {
                HKU_CHECK(block().have(stock), "Stock not belong to block! {}", stock);
            }
        }
    }

    vector<IndicatorList> result;
    const string& driver_type =
      StockManager::instance().getKDataDriverParameter().get<const string&>("type");
    if (driver_type == "clickhouse") {
        result = hku::getValues(*this, stocks, query, align, fill_null, tovalue, align_dates);
        return result;
    }

    // Formula-bearing results must keep independent graphs. The compiled plan is therefore an
    // internal value-only fast path and is not observable through the existing public API.
    if (tovalue) {
        const size_t stk_total = stocks.size();
        const size_t factor_total = m_data->factors.size();
        result.resize(stk_total, IndicatorList(factor_total));
        HKU_IF_RETURN(stk_total == 0 || factor_total == 0, result);

        DatetimeList dates;
        Indicator null_ind;
        if (align) {
            dates = align_dates.empty() ? StockManager::instance().getTradingCalendar(query)
                                        : align_dates;
            HKU_IF_RETURN(dates.empty(), result);
            null_ind = PRICELIST(PriceList(dates.size(), Null<price_t>()), dates);
        }

        IndicatorList formulas;
        formulas.reserve(factor_total);
        for (const auto& factor : m_data->factors) {
            formulas.emplace_back(align ? ALIGN(factor.formula(), dates, fill_null)
                                        : factor.formula());
        }
        const detail::CompiledFactorPlan plan(formulas);
        if (plan.isReusable()) {
            auto calculate_one = [&](detail::FactorPlanExecutor& executor, size_t i) {
                IndicatorList one_result(factor_total);
                KData kdata = stocks[i].getKData(query);
                if (kdata.empty()) {
                    if (align) {
                        std::fill(one_result.begin(), one_result.end(), null_ind);
                    }
                    return one_result;
                }

                return executor.executeValues(kdata);
            };

            // A range task is submitted to the global thread pool directly here, and an outer
            // caller may itself submit this function from a work thread (a nested call).
            // wait_for_all_non_blocking must support work-stealing the submitted subtasks during
            // the waiting (otherwise, when the pool is filled by the outer tasks, it would
            // deadlock: the outer waits for this task, this task waits for the subtasks and no idle
            // worker executes them). Before changing this part,
            auto* task_group = get_global_task_group();
            HKU_ASSERT(task_group);
            auto ranges = parallelIndexRange(0, stk_total, task_group->worker_num());
            vector<std::future<void>> tasks;
            tasks.reserve(ranges.size());
            for (const auto& range : ranges) {
                tasks.emplace_back(task_group->submit([&, range]() {
                    auto executor = plan.createExecutor();
                    for (size_t i = range.first; i < range.second; ++i) {
                        result[i] = calculate_one(executor, i);
                    }
                }));
            }
            wait_for_all_non_blocking(*task_group, tasks);
            for (auto& task : tasks) {
                task.get();
            }
            return result;
        }
    }

    // Create the result container, one IndicatorList per stock
    size_t stk_total = stocks.size();
    size_t factor_total = m_data->factors.size();
    result.resize(stk_total);
    for (size_t i = 0; i < stk_total; ++i) {
        result[i].resize(factor_total);
    }

    const auto& factors = m_data->factors;
    global_parallel_for_index_void(0, factor_total, [&](size_t i) {
        IndicatorList factor_values =
          factors[i].getValues(stocks, query, align, fill_null, tovalue, false, align_dates);
        for (size_t j = 0; j < stk_total; ++j) {
            result[j][i] = std::move(factor_values[j]);
        }
    });

    return result;
}

vector<IndicatorList> FactorSet::getAllValues(const KQuery& query, bool align, bool fill_null,
                                              bool tovalue, const DatetimeList& align_dates) const {
    StockList stocks =
      block().empty() ? StockManager::instance().getStockList() : block().getStockList();
    return getValues(stocks, query, align, fill_null, tovalue, false, align_dates);
}

}  // namespace hku
