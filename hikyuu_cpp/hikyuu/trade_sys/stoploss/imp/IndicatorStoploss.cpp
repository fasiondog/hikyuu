/*
 * IndicatorStoploss.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

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

price_t IndicatorStoploss::getPrice(const Datetime& datetime, price_t price) {
    return m_result.count(datetime) ? m_result[datetime] : 0.0;
}

void IndicatorStoploss::_reset() {
    m_result.clear();
}

StoplossPtr IndicatorStoploss::_clone() {
    auto p = make_shared<IndicatorStoploss>();
    p->m_ind = m_ind;
    p->m_result = m_result;
    return p;
}

void IndicatorStoploss::_calculate() {
    Indicator ind = m_ind(m_kdata);
    size_t total = ind.size();
    auto const* ind_data = ind.data();
    auto const* ks = m_kdata.data();
    for (size_t i = ind.discard(); i < total; ++i) {
        m_result[ks[i].datetime] = ind_data[i];
    }
}

StoplossPtr HKU_API ST_Indicator(const Indicator& ind) {
    return make_shared<IndicatorStoploss>(ind);
}

} /* namespace hku */
