/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SIGNED_POWER_H_
#define INDICATOR_CRT_SIGNED_POWER_H_

#include "CVAL.h"

namespace hku {

/**
 * Power
 * @details
 * <pre>
 * SIGNED_POWER(A,B) returns A to the power of B, but keeps the original sign
 * For example: SIGNED_POWER(CLOSE,3) gives the cube of the close price and keeps the original sign
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API SIGNED_POWER(int n);
Indicator HKU_API SIGNED_POWER(const IndParam& n);

inline Indicator SIGNED_POWER(const Indicator& ind, int n) {
    return SIGNED_POWER(n)(ind);
}

inline Indicator SIGNED_POWER(const Indicator& ind, const IndParam& n) {
    return SIGNED_POWER(n)(ind);
}

inline Indicator SIGNED_POWER(const Indicator& ind, const Indicator& n) {
    return SIGNED_POWER(IndParam(n))(ind);
}

inline Indicator SIGNED_POWER(Indicator::value_t val, int n) {
    return SIGNED_POWER(CVAL(val), n);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SIGNED_POWER_H_ */
