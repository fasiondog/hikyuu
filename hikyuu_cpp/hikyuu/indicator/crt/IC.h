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
 * @brief Calculate the IC of the given factor relative to the reference security (actually RankIC)
 * @note The IC originally needs "the factor value at t -> the return at t+1"; here it is changed to
 *        calculating "the factor value at t -> the return of the N days before t" (such as the
 *        return of the past 5 days), which is called the "current IC". (Otherwise the current
 * values would all be missing NA) If a strict "the factor value at t -> the return at t+1"
 * calculation is needed, please set strict=True (note that in this mode the last n values are NA)
 * @param stks the security portfolio
 * @param n time window (corresponding to the n-day return)
 * @param spearman use the spearman correlation coefficient, otherwise pearson
 * @param strict strict mode, it follows the IC definition "the factor value at t -> the return at
 *               t+1"
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
 * @brief Calculate the IC of the given factor list relative to the given return list,
 * where inds and returns are both already calculated and aligned by date. inds does not need to be
 * shifted right by n.
 * @note It is a numeric calculation only, the returned result has no aligned dates
 * @param inds factor list, inds does not need to be shifted right by n.
 * @param returns return list
 * @param n time window (corresponding to the n-day return)
 * @param use_spearman use the spearman correlation coefficient, otherwise pearson
 * @param strict strict mode, it follows the IC definition "the factor value at t -> the return at
 *               t+1"
 */
Indicator HKU_API IC(IndicatorList inds, IndicatorList returns, int n = 1, bool use_spearman = true,
                     bool strict = false);

}  // namespace hku