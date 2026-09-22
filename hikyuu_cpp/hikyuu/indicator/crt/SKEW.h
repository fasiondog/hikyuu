/*
 * SKEW.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SKEW_H_
#define INDICATOR_CRT_SKEW_H_

#include "../Indicator.h"

namespace hku {

/**
 * Calculate the unadjusted population skewness within N periods
 * @param n N-day time window (greater than or equal to 3 or equal to 0); when it is 0 the actual
 *          length of the input ind is used
 * @ingroup Indicator
 */
Indicator HKU_API SKEW(int n = 10);
Indicator HKU_API SKEW(const IndParam& n);

inline Indicator SKEW(const Indicator& data, int n = 10) {
    return SKEW(n)(data);
}

inline Indicator SKEW(const Indicator& data, const IndParam& n) {
    return SKEW(n)(data);
}

inline Indicator SKEW(const Indicator& data, const Indicator& n) {
    return SKEW(IndParam(n))(data);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SKEW_H_ */