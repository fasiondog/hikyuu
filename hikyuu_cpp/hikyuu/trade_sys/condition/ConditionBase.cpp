/*
 * ConditionBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "ConditionBase.h"

namespace hku {


HKU_API std::ostream & operator<<(std::ostream& os, const ConditionBase& cn) {
    os << "Condition(" << cn.name() << ", " << cn.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const ConditionPtr& cn) {
    if (cn) {
        os << *cn;
    } else {
        os << "Condition(NULL)";
    }
    return os;
}

ConditionBase::ConditionBase(): m_name("ConditionBase") {

}

ConditionBase::ConditionBase(const string& name): m_name(name) {

}

ConditionBase::~ConditionBase() {

}

void ConditionBase::reset() {
    m_valid.clear();
    _reset();
}

ConditionPtr ConditionBase::clone() {
    ConditionPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [ConditionBase::clone]");
        p = ConditionPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [ConditionBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_kdata = m_kdata;
    p->m_valid = m_valid;

    //tm、sg由系统运行时进行设定，不作clone
    //p->m_tm = m_tm->clone();
    //p->m_sg = m_sg->clone();
    return p;
}


void ConditionBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!m_sg) {
        HKU_WARN("m_sg is NULL! [ConditionBase::setTO]");
        return;
    }
    if (!kdata.empty()) {
        _calculate();
    }
}

void ConditionBase::_addValid(const Datetime& datetime) {
    m_valid.insert(datetime);
}

bool ConditionBase::isValid(const Datetime& datetime) {
    if (m_valid.count(datetime) != 0) {
        return true;
    }
    return false;
}

} /* namespace hku */
