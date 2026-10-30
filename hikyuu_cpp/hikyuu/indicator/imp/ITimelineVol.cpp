/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */

#include "../crt/TIMELINE.h"
#include "../crt/TIMELINEVOL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

Indicator HKU_API TIMELINEVOL() {
    Indicator ind = TIMELINE();
    ind.name("TIMELINEVOL");
    ind.setParam<string>("part", "vol");
    return ind;
}

Indicator HKU_API TIMELINEVOL(const KData& k) {
    Indicator ind = TIMELINE(k);
    ind.name("TIMELINEVOL");
    ind.setParam<string>("part", "vol");
    return ind;
}

void registerPart_TIMELINEVOL() {
    registerIndicatorPart("core.ind.timelinevol", [] { return TIMELINEVOL(); });
}

}  // namespace hku