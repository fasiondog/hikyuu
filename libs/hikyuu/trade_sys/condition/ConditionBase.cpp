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
    ConditionPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_kdata = m_kdata;
    p->m_valid = m_valid;
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
