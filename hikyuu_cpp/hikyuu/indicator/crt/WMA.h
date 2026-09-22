/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */

#pragma once
#ifndef WMA_H_
#define WMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * Weighted moving average

 * Algorithm: Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n)

 * @param n the period window for calculating the average, n >= 1

 * @ingroup Indicator
 */
Indicator HKU_API WMA(int n = 22);
Indicator HKU_API WMA(const IndParam& n);

/**
 * Weighted moving average

 * Algorithm: Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n)

 * @param ind the data to be calculated

 * @param n the period window for calculating the average

 * @ingroup Indicator
 */
inline Indicator WMA(const Indicator& ind, int n = 22) {
    return WMA(n)(ind);
}

inline Indicator WMA(const Indicator& ind, const IndParam& n) {
    return WMA(n)(ind);
}

inline Indicator WMA(const Indicator& ind, const Indicator& n) {
    return WMA(IndParam(n))(ind);
}

}  // namespace hku

#endif /* MA_H_ */
