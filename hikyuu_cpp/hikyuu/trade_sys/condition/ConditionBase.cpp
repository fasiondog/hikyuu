/*
 * ConditionBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/PRICELIST.h"
#include "ConditionBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const ConditionBase& cn) {
    os << "Condition(" << cn.name() << ", " << cn.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const ConditionPtr& cn) {
    if (cn) {
        os << *cn;
    } else {
        os << "Condition(NULL)";
    }
    return os;
}

ConditionBase::ConditionBase() : m_name("ConditionBase") {}

ConditionBase::ConditionBase(const string& name) : m_name(name) {}

ConditionBase::~ConditionBase() {}

void ConditionBase::reset() {
    m_date_index.clear();
    m_values.clear();
    _reset();
}

ConditionPtr ConditionBase::clone() {
    ConditionPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = ConditionPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_kdata = m_kdata;
    p->m_date_index = m_date_index;
    p->m_values = m_values;

    // tm、sg由系统运行时进行设定，不作clone
    // p->m_tm = m_tm->clone();
    // p->m_sg = m_sg->clone();
    return p;
}

void ConditionBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        m_date_index.clear();
        size_t total = kdata.size();
        m_values.resize(total);
        auto const* ks = m_kdata.data();
        for (size_t i = 0; i < total; i++) {
            m_values[i] = 0.0;
            m_date_index[ks[i].datetime] = i;
        }
        _calculate();
    }
}

void ConditionBase::_addValid(const Datetime& datetime, price_t value) {
    auto iter = m_date_index.find(datetime);
    HKU_IF_RETURN(iter == m_date_index.end(), void());
    m_values[iter->second] += value;
}

bool ConditionBase::isValid(const Datetime& datetime) {
    auto iter = m_date_index.find(datetime);
    HKU_IF_RETURN(iter == m_date_index.end(), false);
    return m_values[iter->second] > 0.;
}

DatetimeList ConditionBase::getDatetimeList() const {
    DatetimeList result;
    for (const auto& d : m_date_index) {
        if (m_values[d.second] > 0.0) {
            result.emplace_back(d.first);
        }
    }
    return result;
}

Indicator ConditionBase::getValues() const {
    return PRICELIST(m_values);
}

} /* namespace hku */
