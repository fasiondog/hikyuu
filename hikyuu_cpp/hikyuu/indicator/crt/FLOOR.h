/*
 * FLOOR.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_FLOOR_H_
#define INDICATOR_CRT_FLOOR_H_

#include "CVAL.h"

namespace hku {

/**
 * Round down (round in the direction of decreasing value) to an integer
 * Usage: FLOOR(A) returns the nearest integer in the direction of decreasing value of A
 * For example: FLOOR(12.3) gives 12
 * @ingroup Indicator
 */
Indicator HKU_API FLOOR();

inline Indicator FLOOR(const Indicator& ind) {
    return FLOOR()(ind);
}

inline Indicator FLOOR(Indicator::value_t val) {
    return FLOOR(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROUNDUP_H_ */
