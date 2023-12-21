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
    m_valid.clear();
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
    p->m_valid = m_valid;

    // tm、sg由系统运行时进行设定，不作clone
    // p->m_tm = m_tm->clone();
    // p->m_sg = m_sg->clone();
    return p;
}

void ConditionBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

void ConditionBase::_addValid(const Datetime& datetime) {
    m_valid.insert(datetime);
}

bool ConditionBase::isValid(const Datetime& datetime) {
    return m_valid.count(datetime) != 0;
}

DatetimeList ConditionBase::getDatetimeList() const {
    DatetimeList result;
    HKU_IF_RETURN(m_valid.empty(), result);

    for (const auto& d : m_valid) {
        result.emplace_back(d);
    }

    return result;
}

Indicator ConditionBase::getValues() const {
    PriceList values(m_kdata.size());
    DatetimeList ds = m_kdata.getDatetimeList();
    for (size_t i = 0, len = ds.size(); i < len; i++) {
        values[i] = m_valid.count(ds[i]) != 0 ? 1.0 : 0.0;
    }
    return PRICELIST(values);
}

} /* namespace hku */
