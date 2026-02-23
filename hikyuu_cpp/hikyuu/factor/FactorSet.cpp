/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include "FactorSet.h"

namespace hku {

shared_ptr<FactorSet::Data> FactorSet::ms_null_factorset{make_shared<FactorSet::Data>()};

FactorSet::FactorSet() : m_data(ms_null_factorset) {}

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

void FactorSet::addFactor(const Factor& factor) {
    HKU_CHECK(factor.ktype() == m_data->ktype, "ktype not match!");
    HKU_CHECK(factor.block().strongHash() == m_data->block.strongHash(), "block not match!");
    
    const string& factor_name = factor.name();
    
    // 检查是否已存在同名因子
    auto it = m_data->m_nameIndexMap.find(factor_name);
    if (it != m_data->m_nameIndexMap.end()) {
        // 存在同名因子，覆盖之
        size_t index = it->second;
        m_data->m_factors[index] = factor;
    } else {
        // 添加新因子到 vector 末尾
        size_t index = m_data->m_factors.size();
        m_data->m_factors.push_back(factor);
        // 在 map 中记录名称到索引的映射
        m_data->m_nameIndexMap[factor_name] = index;
    }
}

void FactorSet::addFactor(Factor&& factor) {
    HKU_CHECK(factor.ktype() == m_data->ktype, "ktype not match!");
    HKU_CHECK(factor.block().strongHash() == m_data->block.strongHash(), "block not match!");
    
    const string& factor_name = factor.name();
    
    // 检查是否已存在同名因子
    auto it = m_data->m_nameIndexMap.find(factor_name);
    if (it != m_data->m_nameIndexMap.end()) {
        // 存在同名因子，覆盖之
        size_t index = it->second;
        m_data->m_factors[index] = std::move(factor);
    } else {
        // 添加新因子到 vector 末尾
        size_t index = m_data->m_factors.size();
        m_data->m_factors.push_back(std::move(factor));
        // 在 map 中记录名称到索引的映射
        m_data->m_nameIndexMap[factor_name] = index;
    }
}

void FactorSet::removeFactor(const string& name) {
    auto it = m_data->m_nameIndexMap.find(name);
    if (it == m_data->m_nameIndexMap.end()) {
        return; // 因子不存在
    }
    
    size_t index_to_remove = it->second;
    size_t last_index = m_data->m_factors.size() - 1;
    
    // 如果要删除的不是最后一个元素，需要调整后续元素的索引
    if (index_to_remove != last_index) {
        // 将最后一个元素移动到要删除的位置
        m_data->m_factors[index_to_remove] = std::move(m_data->m_factors[last_index]);
        // 更新移动元素在 map 中的索引
        const string& moved_factor_name = m_data->m_factors[index_to_remove].name();
        m_data->m_nameIndexMap[moved_factor_name] = index_to_remove;
    }
    
    // 删除最后一个元素和 map 中的条目
    m_data->m_factors.pop_back();
    m_data->m_nameIndexMap.erase(it);
}

bool FactorSet::hasFactor(const string& name) const noexcept {
    return m_data->m_nameIndexMap.find(name) != m_data->m_nameIndexMap.end();
}

Factor FactorSet::getFactor(const string& name) const {
    auto it = m_data->m_nameIndexMap.find(name);
    HKU_CHECK(it != m_data->m_nameIndexMap.end(), "Factor '{}' not found!", name);
    return m_data->m_factors[it->second];
}

vector<IndicatorList> FactorSet::getValues(const StockList& stocks, const KQuery& query,
                                           bool check) const {
    if (check) {
        const auto& block = this->block();
        if (!block.empty()) {
            for (auto& stock : stocks) {
                HKU_CHECK(block.have(stock), "Stock not belong to block! {}", stock);
            }
        }
    }

    // 创建结果容器，每个股票对应一个 IndicatorList
    vector<IndicatorList> result(stocks.size());

    // 按添加顺序遍历所有因子
    for (const auto& factor : m_data->m_factors) {
        // 获取该因子对所有股票的计算结果
        IndicatorList factor_values = factor.getValues(stocks, query, false);

        // 将结果按股票顺序分配到对应位置
        for (size_t i = 0; i < stocks.size() && i < factor_values.size(); ++i) {
            result[i].push_back(std::move(factor_values[i]));
        }
    }

    return result;
}

}  // namespace hku