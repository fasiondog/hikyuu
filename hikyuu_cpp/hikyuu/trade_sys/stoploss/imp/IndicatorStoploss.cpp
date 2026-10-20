/*
 * IndicatorStoploss.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include <cmath>
#include "../../../indicator/crt/ALIGN.h"
#include "../../../indicator/crt/KDATA.h"
#include "IndicatorStoploss.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IndicatorStoploss)
#endif

namespace hku {

IndicatorStoploss::IndicatorStoploss() : StoplossBase("ST_Indicator") {}

IndicatorStoploss::IndicatorStoploss(const Indicator& op)
: StoplossBase("ST_Indicator"), m_ind(op) {}

IndicatorStoploss::~IndicatorStoploss() {}

price_t IndicatorStoploss::_getPrice(const Datetime& datetime, price_t price) {
    auto it = m_result.find(datetime);
    // A bar without a usable indicator value (warm-up, non-finite or outside the calculated range)
    // is invalid: the system opens the trade without stop-loss
    return it == m_result.end() ? Null<price_t>() : it->second;
}

void IndicatorStoploss::_reset() {
    m_result.clear();
}

StoplossPtr IndicatorStoploss::_clone() {
    auto p = make_shared<IndicatorStoploss>();
    p->m_ind = m_ind.clone();
    return p;
}

void IndicatorStoploss::_calculate() {
    // Align the indicator to the trading object so that ind and m_kdata share the same date
    // range; otherwise an index mismatch would read m_kdata out of bounds.
    Indicator ind = ALIGN(m_ind(m_kdata), m_kdata);
    HKU_WARN_IF_RETURN(ind.size() != m_kdata.size(), void(),
                       "IndicatorStoploss: indicator size({}) does not match kdata size({}), no "
                       "stop-loss will be applied",
                       ind.size(), m_kdata.size());
    size_t total = ind.size();
    auto const* ind_data = ind.data();
    auto const* ks = m_kdata.data();
    for (size_t i = ind.discard(); i < total; ++i) {
        // Drop non-finite values: they have no price meaning, so the bar is left without a usable
        // level and getPrice reports it as invalid
        if (std::isfinite(ind_data[i])) {
            m_result[ks[i].datetime] = ind_data[i];
        }
    }
}

StoplossPtr HKU_API ST_Indicator(const Indicator& ind) {
    return make_shared<IndicatorStoploss>(ind);
}

} /* namespace hku */
