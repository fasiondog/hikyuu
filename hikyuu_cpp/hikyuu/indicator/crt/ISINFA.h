/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ISINFA_H_
#define INDICATOR_CRT_ISINFA_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Whether it is negative infinity (use ISINF for positive infinity)
 * @ingroup Indicator
 */
Indicator HKU_API ISINFA();

inline Indicator ISINFA(const Indicator& ind) {
    return ISINFA()(ind);
}

inline void registerPart_ISINFA() {
    registerIndicatorPart("core.ind.isinfa", [] { return ISINFA(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_ISINFA_H_ */
