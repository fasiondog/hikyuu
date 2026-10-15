/*
 * SlippageBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include <algorithm>
#include "SlippageBase.h"
#include "hikyuu/KRecord.h"
#include "hikyuu/Stock.h"

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
    const price_t price = _getRealBuyPrice(datetime, planPrice);
    if (m_kdata.empty()) {
        return price;
    }
    const KRecord& k = m_kdata.getKRecord(datetime);
    if (k == KRecord::NullKRecord) {
        return price;
    }
    return std::min(price, k.highPrice);
}

price_t SlippageBase::getRealSellPrice(const Datetime& datetime, price_t planPrice) {
    HKU_WARN_IF_RETURN(!(planPrice > 0.0), planPrice, "plan price must be positive");
    const price_t price = _getRealSellPrice(datetime, planPrice);
    if (m_kdata.empty()) {
        return price;
    }
    const KRecord& k = m_kdata.getKRecord(datetime);
    if (k == KRecord::NullKRecord) {
        // Without the day's low, at least keep a non-negative, tick-aligned price
        const Stock& stock = m_kdata.getStock();
        const price_t floor = stock.isNull() ? 0.0 : stock.tick();
        return std::max(price, floor);
    }
    return std::max(price, k.lowPrice);
}

} /* namespace hku */
