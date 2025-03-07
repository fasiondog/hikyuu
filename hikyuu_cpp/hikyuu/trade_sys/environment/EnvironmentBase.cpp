/*
 * Environment.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/PRICELIST.h"
#include "EnvironmentBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const EnvironmentBase& en) {
    os << "Environment(" << en.name() << " " << en.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const EnvironmentPtr& en) {
    if (en) {
        os << *en;
    } else {
        os << "Environment(NULL)";
    }
    return os;
}

EnvironmentBase::EnvironmentBase() : m_name("EnvironmentBase") {}

EnvironmentBase::EnvironmentBase(const EnvironmentBase& base)
: m_params(base.m_params),
  m_name(base.m_name),
  m_query(base.m_query),
  m_date_index(base.m_date_index),
  m_values(base.m_values) {}

EnvironmentBase::EnvironmentBase(const string& name) : m_name(name) {}

EnvironmentBase::~EnvironmentBase() {}

void EnvironmentBase::baseCheckParam(const string& name) const {}
void EnvironmentBase::paramChanged() {}

void EnvironmentBase::reset() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_query = Null<KQuery>();
    m_date_index.clear();
    m_values.clear();
    _reset();
}

EnvironmentPtr EnvironmentBase::clone() {
    EnvironmentPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = EnvironmentPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_query = m_query;
    p->m_date_index = m_date_index;
    p->m_values = m_values;
    return p;
}

void EnvironmentBase::setQuery(const KQuery& query) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (m_query != query) {
        m_query = Null<KQuery>();
        m_date_index.clear();
        m_values.clear();
        _reset();
        m_query = query;
        _calculate();
    }
}

void EnvironmentBase::_addValid(const Datetime& datetime, price_t value) {
    auto iter = m_date_index.find(datetime);
    if (iter == m_date_index.end()) {
        m_date_index[datetime] = m_values.size();
        m_values.push_back(value);
    } else {
        m_values[iter->second] += value;
    }
}

bool EnvironmentBase::isValid(const Datetime& datetime) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    auto iter = m_date_index.find(datetime);
    HKU_IF_RETURN(iter == m_date_index.end(), false);
    return m_values[iter->second] > 0.;
}

price_t EnvironmentBase::getValue(const Datetime& datetime) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    auto iter = m_date_index.find(datetime);
    return iter == m_date_index.end() ? 0. : m_values[iter->second];
}

Indicator EnvironmentBase::getValues() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    DatetimeList dates;
    PriceList values;
    for (const auto& d : m_date_index) {
        dates.emplace_back(d.first);
    }

    values.reserve(dates.size());
    for (const auto& d : dates) {
        values.emplace_back(m_values[m_date_index.at(d)]);
    }

    return PRICELIST(values, dates);
}

} /* namespace hku */
