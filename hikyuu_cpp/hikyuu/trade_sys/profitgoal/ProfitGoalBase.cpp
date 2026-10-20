/*
 * ProfitGoal.cpp
 *
 *  Created on: 2013-3-7
 *      Author: fasiondog
 */

#include "ProfitGoalBase.h"

#include <cmath>

namespace hku {

price_t ProfitGoalBase::_normalizeGoal(price_t goal) {
    // Keep the valid semantics: Null(NaN) = no goal, 0 = sell now (long side), a finite positive
    // value = the target price. A negative or non-finite return is never a valid profit target and
    // would drive the exit comparison wrong (long: exit every bar on a negative value; short:
    // close every bar on +inf), so it is clamped to Null (no goal).
    if (std::isnan(goal) || goal == 0.0 || (std::isfinite(goal) && goal > 0.0)) {
        return goal;
    }
    return Null<price_t>();
}

price_t ProfitGoalBase::getGoal(const Datetime& datetime, price_t price) {
    return _normalizeGoal(_getGoal(datetime, price));
}

price_t ProfitGoalBase::getShortGoal(const Datetime& datetime, price_t price) {
    return _normalizeGoal(_getShortGoal(datetime, price));
}

HKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalBase& pg) {
    os << "ProfitGoal(" << pg.name() << ", " << pg.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalPtr& pg) {
    if (pg) {
        os << *pg;
    } else {
        os << "ProfitGoal(NULL)";
    }
    return os;
}

ProfitGoalBase::ProfitGoalBase() : m_name("ProfitGoalBase") {}

ProfitGoalBase::ProfitGoalBase(const string& name) : m_name(name) {}

ProfitGoalBase::~ProfitGoalBase() {}

void ProfitGoalBase::baseCheckParam(const string& name) const {}
void ProfitGoalBase::paramChanged() {}

void ProfitGoalBase::reset() {
    m_kdata = Null<KData>();
    m_tm.reset();
    _reset();
}

ProfitGoalPtr ProfitGoalBase::clone() {
    ProfitGoalPtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;
    p->m_tm = m_tm;
    // TO is intentionally dropped so the caller must setTO, which re-runs _reset/_calculate and
    // keeps a precompute subclass (e.g. FixedHoldDays) from reusing state built for the original TO
    p->m_kdata = Null<KData>();
    return p;
}

void ProfitGoalBase::setTO(const KData& kdata) {
    HKU_IF_RETURN(kdata == m_kdata, void());
    m_kdata = kdata;
    _reset();
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hku */
