/*
 * FixedPercentSlippage.cpp
 *
 *  Created on: 2016年5月7日
 *      Author: Administrator
 */

#include "FixedPercentSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedPercentSlippage)
#endif

namespace hku {

FixedPercentSlippage::FixedPercentSlippage() : SlippageBase("FixedPercent") {
    setParam<double>("p", 0.001);
}

FixedPercentSlippage::~FixedPercentSlippage() {}

price_t FixedPercentSlippage ::getRealBuyPrice(const Datetime& datetime, price_t price) {
    return price * (1 + getParam<double>("p"));
}

price_t FixedPercentSlippage ::getRealSellPrice(const Datetime& datetime, price_t price) {
    return price * (1 - getParam<double>("p"));
}

void FixedPercentSlippage::_calculate() {}

SlippagePtr HKU_API SP_FixedPercent(double p) {
    FixedPercentSlippage* ptr = new FixedPercentSlippage;
    ptr->setParam("p", p);
    return SlippagePtr(ptr);
}

} /* namespace hku */
