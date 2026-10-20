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

CycleSignal::CycleSignal() : SignalBase("SG_Cycle") {
    setParam<bool>("alternate", false);
    setParam<bool>("cycle", true);
}

void CycleSignal::_checkParam(const string& name) const {
    if ("cycle" == name) {
        bool cycle = getParam<bool>(name);
        HKU_CHECK(cycle, "param cycle must be true!");
    }
    // alternate is not validated here: the base class coerces it back to false for any cycle
    // signal, so the invariant holds without leaving the parameter in a rejected state
}

void CycleSignal::_calculate(const KData& kdata) {
    // A cycle signal only consumes the declared rebalancing days; it never derives them by itself
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    const Datetime& first = kdata[0].datetime;
    const Datetime& last = kdata[total - 1].datetime;
    for (const auto& date : getCycleDates()) {
        if (date >= first && date <= last) {
            _addBuySignal(date);
        }
    }
}

SignalPtr HKU_API SG_Cycle() {
    return make_shared<CycleSignal>();
}

}  // namespace hku