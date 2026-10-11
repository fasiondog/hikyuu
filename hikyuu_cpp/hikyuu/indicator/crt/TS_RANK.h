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
 * Usage: TS_RANK(X,N) means the rank of X within the past N periods divided by the number of the
 * valid (non-NaN) observations in the window
 * For example: TS_RANK(CLOSE,20) means the rank ratio of the close price within the past 20 periods
 *
 * Definition in Alpha101:
 * TS_RANK(x, n) = (rank of x in the last n observations) / n
 * where rank is the ascending rank, i.e. the smaller values are ranked first
 *
 * Implementation description:
 * For every period i, count the number of the non-NaN elements in the window [i-n+1, i] that are
 * less than or equal to x[i] (call it valid_le), and the number of the non-NaN elements in the same
 * window (call it valid). Then TS_RANK = valid_le / valid. If x[i] itself is NaN, TS_RANK is Null;
 * if the window has any valid observation (which is guaranteed when x[i] is not NaN because x[i]
 * itself counts), the result is well-defined.
 *
 * NaN policy: consistent with the library-wide "extreme/mean ignore missing, denominator uses valid
 * count" convention (see MA, HHV, LLV, STDEV). Missing days are excluded from both the numerator
 * and the denominator, so a stock with suspension gaps is ranked against the days it actually
 * traded, not against the calendar slots.
 *
 * Complexity: each bar runs an O(n) linear scan over the window (per bar parallelized across
 * days). Realistic measurement (sh000001 daily, total ~5K bars, cycle=100, macOS arm64 release):
 * n=60 ~0.017 us/bar, n=2000 ~0.079 us/bar; the O(n) cost stays cheap thanks to cache-friendly
 * sequential access and no incremental order-statistic structure is warranted for typical n <= 200.
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