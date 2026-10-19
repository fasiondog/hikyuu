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
    m_calculated = false;
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
    // The clone recalculates lazily so its runtime state is rebuilt on the next setTO instead of
    // keeping the stale snapshot
    p->m_calculated = false;
    return p;
}

void SlippageBase::setTO(const KData& kdata) {
    HKU_IF_RETURN(m_calculated && m_kdata == kdata, void());
    m_kdata = kdata;
    m_calculated = false;
    if (!kdata.empty()) {
        _calculate();
        m_calculated = true;
    }
}

price_t SlippageBase::getRealBuyPrice(const Datetime& datetime, price_t planPrice) {
    HKU_WARN_IF_RETURN(!(planPrice > 0.0), planPrice, "plan price must be positive");
    return _getRealBuyPrice(datetime, planPrice);
}

price_t SlippageBase::getRealSellPrice(const Datetime& datetime, price_t planPrice) {
    HKU_WARN_IF_RETURN(!(planPrice > 0.0), planPrice, "plan price must be positive");
    return _getRealSellPrice(datetime, planPrice);
}

price_t SlippageBase::getRealBuyShortPrice(const Datetime& datetime, price_t planPrice) {
    HKU_WARN_IF_RETURN(!(planPrice > 0.0), planPrice, "plan price must be positive");
    return _getRealBuyShortPrice(datetime, planPrice);
}

price_t SlippageBase::getRealSellShortPrice(const Datetime& datetime, price_t planPrice) {
    HKU_WARN_IF_RETURN(!(planPrice > 0.0), planPrice, "plan price must be positive");
    return _getRealSellShortPrice(datetime, planPrice);
}

price_t SlippageBase::_getRealBuyShortPrice(const Datetime& datetime, price_t planPrice) {
    return _getRealBuyPrice(datetime, planPrice);
}

price_t SlippageBase::_getRealSellShortPrice(const Datetime& datetime, price_t planPrice) {
    return _getRealSellPrice(datetime, planPrice);
}

} /* namespace hku */
