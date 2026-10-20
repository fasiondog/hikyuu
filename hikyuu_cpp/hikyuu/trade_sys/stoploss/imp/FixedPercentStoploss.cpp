/*
 * FixedPercentStoploss.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include <cmath>
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

price_t FixedPercentStoploss::_getPrice(const Datetime& datetime, price_t price) {
    HKU_IF_RETURN(!std::isfinite(price) || price <= 0.0, 0.0);
    double p = getParam<double>("p");
    // A 100% stop-loss can never be reached, i.e. it means no stop-loss
    HKU_IF_RETURN(p == 1.0, 0.0);
    Stock stock = m_kdata.getStock();
    int precision = stock.isNull() ? 2 : stock.precision();
    double factor = std::pow(10.0, precision);
    // Round to the minimum price tick, then clamp down by one tick if it would reach or exceed the
    // buy price, so the stop-loss is never rounded back up to the entry price.
    price_t result = roundEx(price * (1 - p), precision);
    if (result >= price) {
        result = price - 1.0 / factor;
    }
    // The level cannot be represented by the price tick (it rounds to the tick or the one-tick
    // clamp leaves it at or below 0): report it as invalid, and the system opens the trade without
    // stop-loss
    return result > 0.0 ? result : Null<price_t>();
}

price_t FixedPercentStoploss::_getShortPrice(const Datetime& datetime, price_t price) {
    HKU_IF_RETURN(!std::isfinite(price) || price <= 0.0, 0.0);
    Stock stock = m_kdata.getStock();
    int precision = stock.isNull() ? 2 : stock.precision();
    return roundUp(price * (1 + getParam<double>("p")), precision);
}

void FixedPercentStoploss::_calculate() {}

StoplossPtr HKU_API ST_FixedPercent(double p) {
    StoplossPtr result = make_shared<FixedPercentStoploss>();
    result->setParam<double>("p", p);
    return result;
}

} /* namespace hku */
