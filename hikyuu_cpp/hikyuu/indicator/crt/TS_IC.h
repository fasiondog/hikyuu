/*
 * TS_IC.h
 *
 *  Created on: 2026-11-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TS_IC_H_
#define INDICATOR_CRT_TS_IC_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * @brief Time-series information coefficient (IC) of a factor for a single security.
 * @details Unlike the cross-section IC (which correlates the factor and the n-day return across
 *          stocks on each day), TS_IC correlates the factor series with the security's own ic_n-day
 *          return series along the time axis, within a rolling window of n observations. The daily
 *          pair is (factor observed at t-ic_n, return over [t-ic_n, t]), matching the IC
 *          convention; the return is computed internally from the current KData
 *          (ROCP(CLOSE, ic_n)), so only the factor is fed in.
 *
 * @note Spearman (default) ranks the pairs inside each window before correlating and excludes
 *       pairs with a non-finite value; Pearson does not filter, so a non-finite value inside the
 *       window propagates and the whole window yields Null.
 * @note strict = true re-indexes the result by the factor observation date (shift left by ic_n);
 *       the trailing ic_n values become Null because their forward returns are not yet realized.
 *       Keep the default (non-strict) for any live / backtest decision path to avoid look-ahead
 *       bias.
 *
 * @param n the rolling correlation window (>= 2)
 * @param ic_n the ic_n-day return span paired with the factor
 * @param spearman use the spearman correlation coefficient, otherwise pearson
 * @param strict re-index by factor observation date; the last ic_n values are Null
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API TS_IC(int n = 30, int ic_n = 1, bool spearman = true, bool strict = false);

inline Indicator TS_IC(const Indicator& ind, int n = 30, int ic_n = 1, bool spearman = true,
                       bool strict = false) {
    return TS_IC(n, ic_n, spearman, strict)(ind);
}

inline void registerPart_TS_IC() {
    registerIndicatorPart("core.ind.ts_ic", [] { return TS_IC(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_TS_IC_H_ */
