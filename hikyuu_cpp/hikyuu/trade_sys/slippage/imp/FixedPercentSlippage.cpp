/*
 * FixedPercentSlippage.cpp
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#include <cmath>
#include <optional>
#include "FixedPercentSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedPercentSlippage)
#endif

namespace hku {

FixedPercentSlippage::FixedPercentSlippage() : SlippageBase("SP_FixedPercent") {
    setParam<double>("p", 0.001);
    setParam<double>("short_p", 0.001);
}

FixedPercentSlippage::~FixedPercentSlippage() {}

void FixedPercentSlippage::_checkParam(const string& name) const {
    if ("p" == name || "short_p" == name) {
        double p = getParam<double>(name);
        HKU_CHECK(p >= 0.0 && p < 1.0, "SP_FixedPercent: param {} must be in [0, 1), got {}", name,
                  p);
    }
}

price_t FixedPercentSlippage ::_getRealBuyPrice(const Datetime& datetime, price_t price) {
    return price * (1 + getParam<double>("p"));
}

price_t FixedPercentSlippage ::_getRealSellPrice(const Datetime& datetime, price_t price) {
    return price * (1 - getParam<double>("p"));
}

price_t FixedPercentSlippage::_getRealBuyShortPrice(const Datetime& datetime, price_t price) {
    return price * (1 + getParam<double>("short_p"));
}

price_t FixedPercentSlippage::_getRealSellShortPrice(const Datetime& datetime, price_t price) {
    return price * (1 - getParam<double>("short_p"));
}

void FixedPercentSlippage::_calculate() {}

SlippagePtr HKU_API SP_FixedPercent(double p, std::optional<double> short_p) {
    SlippagePtr ptr = make_shared<FixedPercentSlippage>();
    ptr->setParam("p", p);
    ptr->setParam("short_p", short_p.value_or(p));
    return ptr;
}

} /* namespace hku */
