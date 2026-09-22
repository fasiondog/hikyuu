/*
 * SUM.h
 *
 *  Created on: 2019-4-1

 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SUM_H_
#define INDICATOR_CRT_SUM_H_

#include "../Indicator.h"

namespace hku {

/**
 * Calculate the sum. SUM(X,N) sums up X within N periods; when N=0 it starts from the first valid
 * value.

 * @param n N-day time window

 * @ingroup Indicator
 */
Indicator HKU_API SUM(int n = 20);
Indicator HKU_API SUM(const IndParam& n);

/**
 * Calculate the sum. SUM(X,N) sums up X within N periods; when N=0 it starts from the first valid
 * value.

 * @param ind the data to be calculated

 * @param n N-day time window

 * @ingroup Indicator
 */
inline Indicator SUM(const Indicator& ind, int n = 20) {
    return SUM(n)(ind);
}

inline Indicator SUM(const Indicator& ind, const IndParam& n) {
    return SUM(n)(ind);
}

inline Indicator SUM(const Indicator& ind, const Indicator& n) {
    return SUM(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SUM_H_ */
