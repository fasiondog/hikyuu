/*
 * FixedValueSlippage.cpp
 *
 *  Created on: 2016年5月7日
 *      Author: Administrator
 */

#include "FixedValueSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedValueSlippage)
#endif

namespace hku {

FixedValueSlippage::FixedValueSlippage() {
    setParam<double>("value", 0.01);
}

FixedValueSlippage::~FixedValueSlippage() {}

void FixedValueSlippage::_checkParam(const string& name) const {
    if ("p" == name) {
        HKU_ASSERT(getParam<double>(name) >= 0.0);
    }
}

price_t FixedValueSlippage ::getRealBuyPrice(const Datetime& datetime, price_t price) {
    return price + getParam<double>("value");
}

price_t FixedValueSlippage ::getRealSellPrice(const Datetime& datetime, price_t price) {
    return price - getParam<double>("value");
}

void FixedValueSlippage::_calculate() {}

SlippagePtr HKU_API SP_FixedValue(double value) {
    SlippagePtr ptr = make_shared<FixedValueSlippage>();
    ptr->setParam("value", value);
    return ptr;
}

} /* namespace hku */
