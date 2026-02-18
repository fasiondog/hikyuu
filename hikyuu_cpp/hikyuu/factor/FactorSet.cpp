/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include "FactorSet.h"

namespace hku {

FactorSet::FactorSet() : m_data(std::make_shared<Data>()) {}

FactorSet::FactorSet(const string& name, const KQuery::KType& ktype)
: m_data(std::make_shared<Data>()) {
    m_data->name = name;
    m_data->ktype = ktype;
}

FactorSet::FactorSet(const FactorSet& other) : m_data(other.m_data) {}

FactorSet::FactorSet(FactorSet&& other) : m_data(std::move(other.m_data)) {
    other.m_data = std::make_shared<Data>();
}

FactorSet& FactorSet::operator=(const FactorSet& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data = other.m_data;
    return *this;
}

FactorSet& FactorSet::operator=(FactorSet&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_data = std::move(other.m_data);
    other.m_data = std::make_shared<Data>();
    return *this;
}

void FactorSet::addFactor(const FactorMeta& factor) {
    HKU_CHECK(factor.ktype() == m_data->ktype, "ktype not match!");
    m_data->m_factorDict[factor.name()] = factor;
}

void FactorSet::removeFactor(const string& name) {
    m_data->m_factorDict.erase(name);
}

bool FactorSet::hasFactor(const string& name) const noexcept {
    return m_data->m_factorDict.find(name) != m_data->m_factorDict.end();
}

FactorMeta FactorSet::getFactor(const string& name) const {
    auto it = m_data->m_factorDict.find(name);
    HKU_CHECK(it != m_data->m_factorDict.end(), "Factor '{}' not found!", name);
    return it->second;
}

}  // namespace hku