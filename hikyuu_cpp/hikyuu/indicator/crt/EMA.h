/*
 * EMA.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_EMA_H_
#define INDICATOR_CRT_EMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * Exponential Moving Average (EMA)
 * @note Non-finite input (NaN or ±Inf) at bar i holds the previous EMA (yields EMA[i-1]) instead
 *       of poisoning the whole tail; y[i-1] is still unseeded when no finite observation has been
 *       seen yet. Aligned with AGENTS.md "inf poisons every downstream window aggregate" contract.
 * @param n the period window for calculating the average, it must be an integer greater than 0
 * @ingroup Indicator
 */
Indicator HKU_API EMA(int n = 22);
Indicator HKU_API EMA(const IndParam& n);

/**
 * Exponential Moving Average (EMA)
 * @note Non-finite input (NaN or ±Inf) at bar i holds the previous EMA (yields EMA[i-1]); no
 * permanent contamination.
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

#endif /* INDICATOR_CRT_EMA_H_ */
