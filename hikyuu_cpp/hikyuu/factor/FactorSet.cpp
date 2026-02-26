/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include "FactorSet.h"
#include "hikyuu/plugin/factor.h"
#include "hikyuu/plugin/device.h"

namespace hku {

shared_ptr<FactorSet::Data> FactorSet::ms_null_factorset{make_shared<FactorSet::Data>()};

HKU_API std::ostream& operator<<(std::ostream& os, const FactorSet& set) {
    os << set.str();
    return os;
}

string FactorSet::str() const {
    return fmt::format("FactorSet({}, {}, {}, {})", name(), ktype(), size(), block());
}

FactorSet::FactorSet() : m_data(ms_null_factorset) {}

FactorSet::FactorSet(const IndicatorList& inds, const KQuery::KType& ktype)
: m_data(std::make_shared<Data>()) {
    m_data->name = fmt::format("FSET_{}", Datetime::now().ticks());
    m_data->ktype = ktype;
    for (auto& factor : inds) {
        add(factor);
    }
}

FactorSet::FactorSet(const string& name, const KQuery::KType& ktype, const Block& block)
: m_data(std::make_shared<Data>()) {
    string upper_name = name;
    to_upper(upper_name);
    m_data->name = upper_name;
    m_data->ktype = ktype;
    m_data->block = block;
}

FactorSet::FactorSet(const FactorSet& other) : m_data(other.m_data) {}

FactorSet::FactorSet(FactorSet&& other) : m_data(std::move(other.m_data)) {
    other.m_data = ms_null_factorset;
}

FactorSet& FactorSet::operator=(const FactorSet& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data = other.m_data;
    return *this;
}

FactorSet& FactorSet::operator=(FactorSet&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data = std::move(other.m_data);
    other.m_data = ms_null_factorset;
    return *this;
}

void FactorSet::add(const Factor& factor) {
    HKU_CHECK(!factor.isNull(), "Factor is null!");
    HKU_CHECK(factor.ktype() == m_data->ktype, "ktype not match!");
    HKU_CHECK(factor.block().strongHash() == m_data->block.strongHash(), "block not match!");

    const string& factor_name = factor.name();

    // 检查是否已存在同名因子
    auto it = m_data->nameIndexMap.find(factor_name);
    if (it != m_data->nameIndexMap.end()) {
        // 存在同名因子，覆盖之
        size_t index = it->second;
        m_data->factors[index] = factor;
        HKU_WARN("Factor '{}' already exists, it will be overwritten!", factor_name);

    } else {
        // 添加新因子到 vector 末尾
        size_t index = m_data->factors.size();
        m_data->factors.push_back(factor);
        // 在 map 中记录名称到索引的映射
        m_data->nameIndexMap[factor_name] = index;
    }
}

void FactorSet::add(const Indicator& ind) {
    add(Factor(ind.name(), ind, m_data->ktype, "", "", false, Datetime::min(), m_data->block));
}

void FactorSet::remove(const string& name) {
    auto it = m_data->nameIndexMap.find(name);
    if (it == m_data->nameIndexMap.end()) {
        return;  // 因子不存在
    }

    size_t index_to_remove = it->second;
    size_t last_index = m_data->factors.size() - 1;

    // 如果要删除的不是最后一个元素，需要调整后续元素的索引
    if (index_to_remove != last_index) {
        // 将最后一个元素移动到要删除的位置
        m_data->factors[index_to_remove] = std::move(m_data->factors[last_index]);
        // 更新移动元素在 map 中的索引
        const string& moved_factor_name = m_data->factors[index_to_remove].name();
        m_data->nameIndexMap[moved_factor_name] = index_to_remove;
    }

    // 删除最后一个元素和 map 中的条目
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
    // getFactorSet 返回的对象是 Null, Null为全局
    if (!loaded_set.isNull()) {
        m_data = std::move(loaded_set.m_data);
    }
}

vector<IndicatorList> FactorSet::getValues(const StockList& stocks, const KQuery& query, bool align,
                                           bool fill_null, bool tovalue, bool check) const {
    // SPEND_TIME(FactorSet_getValues);
    if (check) {
        const auto& block = this->block();
        if (!block.empty()) {
            for (auto& stock : stocks) {
                HKU_CHECK(block.have(stock), "Stock not belong to block! {}", stock);
            }
        }
    }

    vector<IndicatorList> result;
    if (isValidLicense()) {
        result = hku::getValues(*this, stocks, query, align, fill_null, tovalue);
        return result;
    }

    // 创建结果容器，每个股票对应一个 IndicatorList
    size_t stk_total = stocks.size();
    size_t factor_total = m_data->factors.size();
    result.resize(stk_total);
    for (size_t i = 0; i < stk_total; ++i) {
        result[i].resize(factor_total);
    }

    const auto& factors = m_data->factors;
    global_parallel_for_index_void(0, factor_total, [&](size_t i) {
        IndicatorList factor_values =
          factors[i].getValues(stocks, query, align, fill_null, tovalue);
        for (size_t j = 0; j < stk_total; ++j) {
            result[j][i] = std::move(factor_values[j]);
        }
    });

    return result;
}

vector<IndicatorList> FactorSet::getAllValues(const KQuery& query, bool align, bool fill_null,
                                              bool tovalue) const {
    StockList stocks =
      block().empty() ? StockManager::instance().getStockList() : block().getStockList();
    return getValues(stocks, query, align, fill_null, tovalue);
}

}  // namespace hku