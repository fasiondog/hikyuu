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

void FixedPercentStoploss::_checkParam(const string& name) const {
    if ("p" == name) {
        double p = getParam<double>("p");
        HKU_ASSERT(p > 0.0 && p <= 1.0);
    }
}

price_t FixedPercentStoploss ::getPrice(const Datetime& datetime, price_t price) {
    Stock stock = m_kdata.getStock();
    int precision = stock.isNull() ? 2 : stock.precision();
    return roundEx(price * (1 - getParam<double>("p")), precision);
}

void FixedPercentStoploss::_calculate() {}

StoplossPtr HKU_API ST_FixedPercent(double p) {
    StoplossPtr result = make_shared<FixedPercentStoploss>();
    result->setParam<double>("p", p);
    return result;
}

} /* namespace hku */
