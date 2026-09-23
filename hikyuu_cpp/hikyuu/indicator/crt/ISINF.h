/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ISINF_H_
#define INDICATOR_CRT_ISINF_H_

#include "../Indicator.h"

namespace hku {

/**
 * Whether it is positive infinity (use ISINFA for negative infinity)
 * @ingroup Indicator
 */
Indicator HKU_API ISINF();

inline Indicator ISINF(const Indicator& ind) {
    return ISINF()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ISINF_H_ */
