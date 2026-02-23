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
    m_data->m_factorDict[factor.name()] = factor;
}

void FactorSet::addFactor(Factor&& factor) {
    HKU_CHECK(factor.ktype() == m_data->ktype, "ktype not match!");
    HKU_CHECK(factor.block().strongHash() == m_data->block.strongHash(), "block not match!");
    m_data->m_factorDict[factor.name()] = std::move(factor);
}

void FactorSet::removeFactor(const string& name) {
    m_data->m_factorDict.erase(name);
}

bool FactorSet::hasFactor(const string& name) const noexcept {
    return m_data->m_factorDict.find(name) != m_data->m_factorDict.end();
}

Factor FactorSet::getFactor(const string& name) const {
    auto it = m_data->m_factorDict.find(name);
    HKU_CHECK(it != m_data->m_factorDict.end(), "Factor '{}' not found!", name);
    return it->second;
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

    // 遍历所有因子
    for (const auto& factor_pair : m_data->m_factorDict) {
        const Factor& factor = factor_pair.second;
        // 获取该因子对所有股票的计算结果
        IndicatorList factor_values = factor.getValues(stocks, query, false);

        // 将结果按股票顺序分配到对应位置
        for (size_t i = 0; i < stocks.size(); ++i) {
            result[i].push_back(std::move(factor_values[i]));
        }
    }

    return result;
}

}  // namespace hku