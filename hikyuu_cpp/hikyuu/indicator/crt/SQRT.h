/*
 * SQRT.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SQRT_H_
#define INDICATOR_CRT_SQRT_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Square root
 * @details
 * <pre>
 * Usage: SQRT(X) is the square root of X
 * For example: SQRT(CLOSE) is the square root of the close price
 * </pre>
 * @note Undefined domain (X < 0) returns null
 * @ingroup Indicator
 */
Indicator HKU_API SQRT();

inline Indicator SQRT(const Indicator& ind) {
    return SQRT()(ind);
}

inline Indicator SQRT(Indicator::value_t val) {
    return SQRT(CVAL(val));
}

inline void registerPart_SQRT() {
    registerIndicatorPart("core.ind.sqrt", [] { return SQRT(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_SQRT_H_ */
