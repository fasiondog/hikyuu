/*
 * FixedPercentStoploss.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "FixedPercentStoploss.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedPercentStoploss)
#endif

namespace hku {

FixedPercentStoploss::FixedPercentStoploss() : StoplossBase("ST_FixedPercent") {
    setParam<double>("p", 0.03);
}

FixedPercentStoploss::~FixedPercentStoploss() {}

price_t FixedPercentStoploss ::getPrice(const Datetime& datetime, price_t price) {
    Stock stock = m_kdata.getStock();
    int precision = stock.isNull() ? 2 : stock.precision();
    return roundEx(price * (1 - getParam<double>("p")), precision);
}

void FixedPercentStoploss::_calculate() {}

StoplossPtr HKU_API ST_FixedPercent(double p) {
    StoplossPtr result(new FixedPercentStoploss());
    result->setParam<double>("p", p);
    return result;
}

} /* namespace hku */
