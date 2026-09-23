/*
 * MA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#pragma once
#ifndef MA_H_
#define MA_H_

#include "../Indicator.h"

namespace hku {

/**
 * Simple moving average
 * @param n the period window for calculating the average; when n is 0 the calculation starts from
 *          the first valid data
 * @ingroup Indicator
 */
Indicator HKU_API MA(int n = 22);
Indicator HKU_API MA(const IndParam& n);

/**
 * Simple moving average
 * @param ind the data to be calculated
 * @param n the period window for calculating the average; when n is 0 the calculation starts from
 *          the first valid data
 * @ingroup Indicator
 */
inline Indicator MA(const Indicator& ind, int n = 22) {
    return MA(n)(ind);
}

inline Indicator MA(const Indicator& ind, const IndParam& n) {
    return MA(n)(ind);
}

inline Indicator MA(const Indicator& ind, const Indicator& n) {
    return MA(IndParam(n))(ind);
}

}  // namespace hku

#endif /* MA_H_ */
