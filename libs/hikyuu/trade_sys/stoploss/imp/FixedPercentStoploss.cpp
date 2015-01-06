/*
 * FixedPercentStoploss.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "FixedPercentStoploss.h"

namespace hku {

FixedPercentStoploss::FixedPercentStoploss(double p)
: StoplossBase("FixedPercent") {
    addParam<double>("p", p);
    if (p < 0.0 || p > 1.0) {
        HKU_ERROR("Invalid params! (0 <= p <= 1 " << m_params
                << " [FixedPercentStoploss::FixedPercentStoploss]" );
    }
}

FixedPercentStoploss::~FixedPercentStoploss() {

}

price_t FixedPercentStoploss
::getPrice(const Datetime& datetime, price_t price) {
    Stock stock = m_kdata.getStock();
    int precision = stock.isNull() ? 2 : stock.precision();
    return roundEx(price * (1 - getParam<double>("p")), precision);
}

StoplossPtr FixedPercentStoploss::_clone() {
    double p = getParam<double>("p");
    return StoplossPtr(new FixedPercentStoploss(p));
}

void FixedPercentStoploss::_reset() {

}

void FixedPercentStoploss::_calculate() {

}


StoplossPtr HKU_API FixedPercent_SL(double p) {
    return StoplossPtr(new FixedPercentStoploss(p));
}


} /* namespace hku */
