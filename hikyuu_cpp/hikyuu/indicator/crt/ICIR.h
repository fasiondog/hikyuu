/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "MA.h"
#include "STDEV.h"

namespace hku {

/**
 * @brief Calculate the ICIR of the given factor relative to the reference security (actually
 * RankIC)
 * @details IR: Information Ratio (IR for short) =
 * the multi-period mean of the IC / the standard deviation of the IC, it represents the ability of
 * the factor to obtain a stable Alpha.
 * @note The IC originally needs "the factor value at t -> the return at t+1"; here it is changed
 *       to calculating "the factor value at t -> the return of the N
 *       days before t" (such as the return of the past 5 days), which is called the "current IC".
 *       (Otherwise the current values would all be missing NA)
 *       If a strict "the factor value at t -> the return at t+1" calculation is needed, please set
 *       strict=True (note that in this mode the last n values are NA)
 * @param ind factor formula
 * @param stks the security portfolio
 * @param query query condition
 * @param n the N-day return corresponding to the IC
 * @param rolling_n the rolling time window
 * @param spearman use the spearman correlation coefficient, otherwise pearson
 * @param strict whether it is the strict mode
 * @return Indicator
 * @ingroup Indicator
 */
inline Indicator ICIR(const Indicator& ind, const StockList& stks, int n = 1, int rolling_n = 120,
                      bool spearman = true, bool strict = false) {
    Indicator ic = IC(ind, stks, n, spearman, strict);
    Indicator x = MA(ic, rolling_n) / STDEV(ic, rolling_n);
    x.name("ICIR");
    x.setParam<int>("n", n);
    x.setParam<int>("rolling_n", rolling_n);
    return x;
}

inline Indicator ICIR(const Indicator& ind, const Block& blk, int n = 1, int rolling_n = 120,
                      bool spearman = true, bool strict = false) {
    Indicator ic = IC(ind, blk, n, spearman, strict);
    Indicator x = MA(ic, rolling_n) / STDEV(ic, rolling_n);
    x.name("ICIR");
    x.setParam<int>("n", n);
    x.setParam<int>("rolling_n", rolling_n);
    return x;
}

}  // namespace hku