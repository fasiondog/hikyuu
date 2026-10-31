/*
 * ABS.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ABS_H_
#define INDICATOR_CRT_ABS_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Calculate the absolute value
 * @ingroup Indicator
 */
Indicator HKU_API ABS();

inline Indicator ABS(const Indicator& ind) {
    return ABS()(ind);
}

inline Indicator ABS(Indicator::value_t val) {
    return ABS(CVAL(val));
}

inline void registerPart_ABS() {
    registerIndicatorPart("core.ind.abs", [] { return ABS(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_ABS_H_ */
