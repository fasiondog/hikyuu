/*
 * KURT.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_KURT_H_
#define INDICATOR_CRT_KURT_H_

#include "../Indicator.h"

namespace hku {

/**
 * Calculate the excess kurtosis within N periods (unadjusted population kurtosis - 3)
 * @param n N-day time window (>= 4); when it is 0 an expanding window is used, i.e. dst[i] is the
 *          excess kurtosis over [discard(), i] (discard becomes discard()+3, first three nan)
 * @ingroup Indicator
 */
Indicator HKU_API KURT(int n = 10);
Indicator HKU_API KURT(const IndParam& n);

inline Indicator KURT(const Indicator& data, int n = 10) {
    return KURT(n)(data);
}

inline Indicator KURT(const Indicator& data, const IndParam& n) {
    return KURT(n)(data);
}

inline Indicator KURT(const Indicator& data, const Indicator& n) {
    return KURT(IndParam(n))(data);
}

}  // namespace hku

#endif /* INDICATOR_CRT_KURT_H_ */