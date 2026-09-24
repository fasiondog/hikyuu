/*
 * EMA.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#pragma once
#ifndef EMA_H_
#define EMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * Exponential Moving Average (EMA)
 * @param n the period window for calculating the average, it must be an integer greater than 0
 * @ingroup Indicator
 */
Indicator HKU_API EMA(int n = 22);
Indicator HKU_API EMA(const IndParam& n);

/**
 * Exponential Moving Average (EMA)
 * @param data the source data to be calculated
 * @param n the period window for calculating the average, it must be an integer greater than 0
 * @ingroup Indicator
 */
inline Indicator EMA(const Indicator& data, int n = 22) {
    return EMA(n)(data);
}

inline Indicator EMA(const Indicator& data, const IndParam& n) {
    return EMA(n)(data);
}

inline Indicator EMA(const Indicator& data, const Indicator& n) {
    return EMA(IndParam(n))(data);
}

}  // namespace hku

#endif /* EMA_H_ */
