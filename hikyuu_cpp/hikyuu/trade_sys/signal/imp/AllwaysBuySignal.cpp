/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "AllwaysBuySignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::AllwaysBuySignal)
#endif

namespace hku {

AllwaysBuySignal::AllwaysBuySignal() : SignalBase("SG_AllwaysBuy") {
    setParam<bool>("alternate", false);
}

void AllwaysBuySignal::_checkParam(const string& name) const {
    if ("alternate" == name) {
        bool alternate = getParam<bool>(name);
        HKU_CHECK(!alternate, "param alternate must be false!");
    }
}

void AllwaysBuySignal::_calculate(const KData& kdata) {
    for (auto iter = kdata.cbegin(); iter != kdata.cend(); ++iter) {
        _addBuySignal(iter->datetime);
    }
}

SignalPtr HKU_API SG_AllwaysBuy() {
    return SignalPtr(new AllwaysBuySignal);
}

}  // namespace hku