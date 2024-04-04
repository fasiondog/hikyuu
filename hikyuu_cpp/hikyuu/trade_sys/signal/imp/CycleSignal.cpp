/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "CycleSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::CycleSignal)
#endif

namespace hku {

CycleSignal::CycleSignal() : SignalBase("SG_AllwaysBuy") {
    setParam<bool>("cycle", true);
}

void CycleSignal::_checkParam(const string& name) const {
    if ("cycle" == name) {
        bool cycle = getParam<bool>(name);
        HKU_CHECK(cycle, "param cycle must be true!");
    }
}

void CycleSignal::_calculate(const KData& kdata) {
    _addBuySignal(getCycleStart());
}

SignalPtr HKU_API SG_Cycle() {
    return make_shared<CycleSignal>();
}

}  // namespace hku