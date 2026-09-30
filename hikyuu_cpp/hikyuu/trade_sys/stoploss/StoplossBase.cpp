/*
 * StoplossBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "StoplossBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const StoplossBase& sl) {
    os << "Stoploss(" << sl.name() << ", " << sl.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const StoplossPtr& sl) {
    if (sl) {
        os << *sl;
    } else {
        os << "Stoploss(NULL)";
    }
    return os;
}

StoplossBase::StoplossBase() : m_name("StoplossBase") {}

StoplossBase::StoplossBase(const string& name) : m_name(name) {}

StoplossBase::~StoplossBase() {}

void StoplossBase::baseCheckParam(const string& name) const {}
void StoplossBase::paramChanged() {}

void StoplossBase::reset() {
    m_kdata = Null<KData>();
    m_tm.reset();
    _reset();
}

StoplossPtr StoplossBase::clone() {
    StoplossPtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

    p->m_is_python_object = m_is_python_object;
    p->m_name = m_name;
    p->m_params = m_params;
    // p->m_tm = m_tm;
    p->m_kdata = m_kdata;
    return p;
}

void StoplossBase::setTO(const KData& kdata) {
    HKU_IF_RETURN(m_kdata == kdata, void());
    m_kdata = kdata;
    _reset();
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hku */
