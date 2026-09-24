/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Original RSRS (underlying β) indicator, based on the rolling N-day OLS regression

 * Formula: High = α + β · Low

 *
 * Every K-line contributes a coordinate point (Low[i], High[i]), and the N points within the rolling
 * window are used for the OLS regression.

 * β is the most original RSRS slope, representing the strength of the support and resistance.

 * Defect: the β center fluctuates greatly in different market ranges, so it cannot be compared
 * directly across the periods.

 *
 * @param n the rolling window, 20 by default

 * @param kdata K-line data

 * @ingroup Indicator
 */
Indicator HKU_API RSRS_BETA(int n = 20);
Indicator HKU_API RSRS_BETA(const KData& kdata, int n = 20);

}  // namespace hku