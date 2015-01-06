/*
 * IndicatorStoploss.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include "../../../indicator/crt/KDATA.h"
#include "IndicatorStoploss.h"

namespace hku {

IndicatorStoploss::IndicatorStoploss(): StoplossBase("IndicatorSL") {

}

IndicatorStoploss::
IndicatorStoploss(const Indicator& ind, const string& kdata_part)
: StoplossBase("IndicatorSL"), m_ind(ind), m_kdata_part(kdata_part) {

}

IndicatorStoploss::~IndicatorStoploss() {

}

price_t IndicatorStoploss::getPrice(const Datetime& datetime, price_t price) {
    return m_result.count(datetime) ? m_result[datetime] : 0.0;
}

void IndicatorStoploss::_reset() {
    m_result.clear();
}

StoplossPtr IndicatorStoploss::_clone() {
    IndicatorStoploss *p = new IndicatorStoploss(m_ind, m_kdata_part);
    p->m_result = m_result;
    return StoplossPtr(p);
}

void IndicatorStoploss::_calculate() {
    Indicator ind = m_ind(KDATA_PART(m_kdata, m_kdata_part));
    size_t total = ind.size();
    for (size_t i = ind.discard(); i < total; ++i) {
        m_result[m_kdata[i].datetime] = ind[i];
    }
}

} /* namespace hku */
