/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ISINF_H_
#define INDICATOR_CRT_ISINF_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Whether it is positive infinity (use ISINFA for negative infinity)
 * @ingroup Indicator
 */
Indicator HKU_API ISINF();

inline Indicator ISINF(const Indicator& ind) {
    return ISINF()(ind);
}

inline void registerPart_ISINF() {
    registerIndicatorPart("core.ind.isinf", [] { return ISINF(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_ISINF_H_ */
