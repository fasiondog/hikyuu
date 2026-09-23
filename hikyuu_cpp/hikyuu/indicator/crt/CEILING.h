/*
 * CEILING.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_CEIL_H_
#define INDICATOR_CRT_CEIL_H_

#include "CVAL.h"

namespace hku {

/**
 * Round up (round in the direction of increasing value)
 * Usage: CEILING(A) returns the nearest integer in the direction of increasing value of A
 * For example: CEILING(12.3) gives 13; CEILING(-3.5) gives -3
 * @ingroup Indicator
 */
Indicator HKU_API CEILING();

inline Indicator CEILING(const Indicator& ind) {
    return CEILING()(ind);
}

inline Indicator CEILING(Indicator::value_t val) {
    return CEILING(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_CEIL_H_ */
