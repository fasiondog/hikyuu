/*
 * StoplossBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "StoplossBase.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream& os, const StoplossBase& sl) {
    os << "Stoploss(" << sl.name() << ", " << sl.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator <<(std::ostream& os, const StoplossPtr& sl) {
    if (sl) {
        os << *sl;
    } else {
        os << "Stoploss(NULL)";
    }
    return os;
}

StoplossBase::StoplossBase(): m_name("StoplossBase") {

}

StoplossBase::StoplossBase(const string& name) : m_name(name) {

}

StoplossBase::~StoplossBase() {

}


StoplossPtr StoplossBase::clone() {
    StoplossPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [StoplossBase::clone]");
        p = StoplossPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [StoplossBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;
    //p->m_tm = m_tm;
    p->m_kdata = m_kdata;
    return p;
}


void StoplossBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hku */
