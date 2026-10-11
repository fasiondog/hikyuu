/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-09
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/Block.h"
#include "../Indicator.h"

namespace hku {

/**
 * @brief Cross-section information coefficient (IC) of a factor over a stock set.
 * @details Each trading day, the factor values and the n-day return of every stock are collected
 *          into a cross-section and their Spearman (default) or Pearson correlation is computed.
 *
 * @note Alignment: the daily pair is (factor observed at t - n, return over [t - n, t]). By
 *       default the result is stored at index t (the observation completion date), so the current
 *       value is readable without future data — the "current IC" convention. Set
 *       strict = true to re-index by the factor observation date (t - n), matching the
 *       industry-standard forward IC definition; in that mode the trailing n values are Null
 *       because their forward returns are not yet realized.
 *
 * @note Choosing strict: backtests that feed IC into MF_ICWeight / MF_ICIRWeight or any
 *       live-decision path should keep the default non-strict mode — strict IC[t] uses
 *       returns over [t, t+n] and would introduce look-ahead bias if consumed at t. Use
 *       strict = true only for offline factor analysis or when aligning with published
 *       (academic / industry) IC numbers that are indexed by factor date.
 * @param stks the security portfolio
 * @param n time window (corresponding to the n-day return)
 * @param spearman use the spearman correlation coefficient, otherwise pearson
 * @param strict re-index by factor observation date; the last n values are Null
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API IC(const StockList& stks, int n = 1, bool spearman = true, bool strict = false);

Indicator HKU_API IC(const Block& blk, int n = 1, bool spearman = true, bool strict = false);

inline Indicator IC(const Indicator& ind, const StockList& stks, int n = 1, bool spearman = true,
                    bool strict = false) {
    return IC(stks, n, spearman, strict)(ind);
}

inline Indicator IC(const Indicator& ind, const Block& blk, int n = 1, bool spearman = true,
                    bool strict = false) {
    return IC(blk, n, spearman, strict)(ind);
}

/**
 * @brief Cross-section IC of an already-aligned factor/return list pair.
 * @details inds and returns are both pre-computed and date-aligned by the caller; inds is
 *          interpreted as the factor at its observation date, no REF shift is applied.
 * @note Numeric-only result, no datetime alignment on the output.
 * @param inds factor list (do not pre-shift by n)
 * @param returns return list
 * @param n time window (corresponding to the n-day return)
 * @param use_spearman use the spearman correlation coefficient, otherwise pearson
 * @param strict re-index by factor observation date; the last n values are Null
 */
Indicator HKU_API IC(IndicatorList inds, IndicatorList returns, int n = 1, bool use_spearman = true,
                     bool strict = false);

}  // namespace hku