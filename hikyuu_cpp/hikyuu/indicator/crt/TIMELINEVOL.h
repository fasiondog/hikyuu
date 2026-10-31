/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TIMELINEVOL_H_
#define INDICATOR_CRT_TIMELINEVOL_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

Indicator HKU_API TIMELINEVOL();
Indicator HKU_API TIMELINEVOL(const KData&);

inline void registerPart_TIMELINEVOL() {
    registerIndicatorPart("core.ind.timelinevol", [] { return TIMELINEVOL(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_TIMELINEVOL_H_ */
