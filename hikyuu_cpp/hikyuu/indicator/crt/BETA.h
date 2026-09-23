/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Calculate the Beta coefficient, such as measuring the sensitivity between the asset return and
 * the market return

 * Beta = Cov(stock_return, market_return) / Var(market_return)
 *
 * @note BETA itself does not perform the return conversion (pct_change) on the input data,

 *       the input indicators should be the already calculated return data.

 * @param ind1 the input indicator, such as the stock return indicator

 * @param ind2 the reference indicator, such as the market return indicator

 * @param n the rolling window (greater than 2 or equal to 0); when it is 0 the actual length of the
 *          input ind is used.

 * @param fill_null fill the missing dates with nan when the dates are aligned

 * @ingroup Indicator
 */
Indicator HKU_API BETA(const Indicator& ind1, const Indicator& ind2, int n = 10,
                       bool fill_null = true);
Indicator HKU_API BETA(const Indicator& ref_ind, int n = 10, bool fill_null = true);

}  // namespace hku