/*
 * StoplossBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include <cmath>
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
void StoplossBase::paramChanged() {
    m_calculated = false;
}

void StoplossBase::reset() {
    m_kdata = Null<KData>();
    m_tm.reset();
    _reset();
    m_calculated = false;
}

StoplossPtr StoplossBase::clone() {
    StoplossPtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

    p->m_is_python_object = m_is_python_object;
    p->m_name = m_name;
    p->m_params = m_params;
    // m_tm and m_kdata are intentionally not copied; the System re-injects them (readyForRun / setTO)
    p->m_calculated = false;
    return p;
}

void StoplossBase::setTO(const KData& kdata) {
    HKU_IF_RETURN(m_calculated && m_kdata == kdata, void());
    m_kdata = kdata;
    _reset();
    m_calculated = false;
    if (!kdata.empty()) {
        _calculate();
        m_calculated = true;
    }
}

price_t StoplossBase::getPrice(const Datetime& datetime, price_t price) {
    price_t result = _getPrice(datetime, price);
    // 0 means no stop-loss; any other non-finite or negative value is invalid and marked as Null,
    // which the system treats as no stop-loss (the trade still takes place)
    HKU_IF_RETURN(result == 0.0, 0.0);
    return std::isfinite(result) && result > 0.0 ? result : Null<price_t>();
}

price_t StoplossBase::getShortPrice(const Datetime& datetime, price_t price) {
    // Same contract as getPrice: invalid results are marked as Null (no stop-loss for the system)
    price_t result = _getShortPrice(datetime, price);
    HKU_IF_RETURN(result == 0.0, 0.0);
    return std::isfinite(result) && result > 0.0 ? result : Null<price_t>();
}

} /* namespace hku */
