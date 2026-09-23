/*
 * TS_RANK.h
 *
 *  Created on: 2026-6-9
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TS_RANK_H_
#define INDICATOR_CRT_TS_RANK_H_

#include "../Indicator.h"

namespace hku {

/**
 * Time series rank, it calculates the rank ratio of the current value within the past N periods
 * @details
 * <pre>
 * Usage: TS_RANK(X,N) means the rank of X within the past N periods (from 1 to N) divided by N
 * For example: TS_RANK(CLOSE,20) means the rank ratio of the close price within the past 20 periods
 *
 * Definition in Alpha101:
 * TS_RANK(x, n) = (rank of x in the last n observations) / n
 * where rank is the ascending rank, i.e. the smaller values are ranked first
 *
 * Implementation description:
 * For every period i, count the number of the elements in the window [i-n+1, i] that are less than
 * or equal to x[i], then TS_RANK = count / n
 * </pre>
 * @param n number of the periods
 * @ingroup Indicator
 */
Indicator HKU_API TS_RANK(int n = 20);
Indicator HKU_API TS_RANK(const IndParam& n);

/**
 * Time series rank, it calculates the rank ratio of the current value within the past N periods
 * @details
 * <pre>
 * Usage: TS_RANK(X,N) means the rank of X within the past N periods (from 1 to N) divided by N
 * For example: TS_RANK(CLOSE,20) means the rank ratio of the close price within the past 20 periods
 * </pre>
 * @param ind the data to be calculated
 * @param n number of the periods
 * @ingroup Indicator
 */
inline Indicator TS_RANK(const Indicator& ind, int n = 20) {
    return TS_RANK(n)(ind);
}

inline Indicator TS_RANK(const Indicator& ind, const IndParam& n) {
    return TS_RANK(n)(ind);
}

inline Indicator TS_RANK(const Indicator& ind, const Indicator& n) {
    return TS_RANK(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_TS_RANK_H_ */