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
 * RSRS right-skewed standard score (level 4 correction)
 *
 * Outputs four result series: bull = z * r2 * beta, beta, r2, z.
 *
 * @note Bars with non-finite Low/High are excluded from the regression; the whole window is Null
 *       when fewer than two finite pairs remain, and the result never returns ±Inf
 *
 * @ingroup Indicator
 */
Indicator HKU_API RSRS_BULL(int n = 20, int m = 60);
Indicator HKU_API RSRS_BULL(const KData& kdata, int n = 20, int m = 60);

}  // namespace hku