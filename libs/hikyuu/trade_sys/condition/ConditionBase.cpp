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

ConditionPtr ConditionBase::clone() {
    ConditionPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_kdata = m_kdata;
    return p;
}


void ConditionBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hku */
