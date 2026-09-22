/*
 * COUNT.h
 *
 *  Created on: 2019-3-25
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_COUNT_H_
#define INDICATOR_CRT_COUNT_H_

#include "../Indicator.h"

namespace hku {

/**
 * Count the total number; count the number of the periods satisfying the condition.
 * @details
 * <pre>
 * Usage: COUNT(X,N) counts the number of the periods satisfying the X condition within N periods;
 * if N=0 it starts from the first valid value.
 * For example: COUNT(CLOSE>OPEN,20) counts the number of the periods closing up within 20 periods
 * </pre>
 * @param n number of the periods
 * @ingroup Indicator
 */
Indicator HKU_API COUNT(int n = 20);
Indicator HKU_API COUNT(const IndParam& n);

/**
 * Count the total number; count the number of the periods satisfying the condition.
 * @details
 * <pre>
 * Usage: COUNT(X,N) counts the number of the periods satisfying the X condition within N periods;
 * if N=0 it starts from the first valid value.
 * For example: COUNT(CLOSE>OPEN,20) counts the number of the periods closing up within 20 periods
 * </pre>
 * @param ind the indicator to be counted
 * @param n number of the periods
 * @ingroup Indicator
 */
inline Indicator COUNT(const Indicator& ind, int n = 20) {
    return COUNT(n)(ind);
}

inline Indicator COUNT(const Indicator& ind, const IndParam& n) {
    return COUNT(n)(ind);
}

inline Indicator COUNT(const Indicator& ind, const Indicator& n) {
    return COUNT(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_COUNT_H_ */
