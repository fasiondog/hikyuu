/*
 * ProfitGoal.cpp
 *
 *  Created on: 2013-3-7
 *      Author: fasiondog
 */

#include "ProfitGoalBase.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const ProfitGoalBase& pg) {
    os << "ProfitGoal(" << pg.name() << ", " << pg.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const ProfitGoalPtr& pg) {
    if (pg) {
        os << *pg;
    } else {
        os << "ProfitGoal(NULL)";
    }
    return os;
}

ProfitGoalBase::ProfitGoalBase(const string& name): m_name(name) {

}

ProfitGoalBase::~ProfitGoalBase() {

}

ProfitGoalPtr ProfitGoalBase::clone() {
    ProfitGoalPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_tm = m_tm;
    p->m_kdata = m_kdata;
    return p;
}


void ProfitGoalBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hku */
