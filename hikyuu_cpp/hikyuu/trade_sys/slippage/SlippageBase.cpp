/*
 * SlippageBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "SlippageBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const SlippageBase& sp) {
    os << "Slippage(" << sp.name() << ", " << sp.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const SlippagePtr& sp) {
    if (sp) {
        os << *sp;
    } else {
        os << "Slippage(NULL)";
    }
    return os;
}

SlippageBase::SlippageBase() : m_name("SlippageBase") {}

SlippageBase::SlippageBase(const string& name) : m_name(name) {}

void SlippageBase::baseCheckParam(const string& name) const {}
void SlippageBase::paramChanged() {}

void SlippageBase::reset() {
    m_kdata = Null<KData>();
    _reset();
}

SlippagePtr SlippageBase::clone() {
    SlippagePtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;
    p->m_kdata = m_kdata;
    return p;
}

void SlippageBase::setTO(const KData& kdata) {
    HKU_IF_RETURN(m_kdata == kdata, void());
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hku */
