/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Calculate the sample correlation coefficient and covariance. In the returned result sets, the
 * first is the correlation coefficient and the second is the covariance
 * @param ind1 indicator 1
 * @param ind2 indicator 2
 * @param n the rolling window (>= 2); when it is 0 an expanding window is used, i.e. dst[i] is
 *          the Pearson corr & sample cov over [discard(), i] (discard becomes 1, bar 0 is nan).
 * @param fill_null fill the missing dates with nan when the dates are aligned
 * @ingroup Indicator
 */
Indicator HKU_API CORR(const Indicator& ind1, const Indicator& ind2, int n = 10,
                       bool fill_null = true);
Indicator HKU_API CORR(const Indicator& ref_ind, int n = 10, bool fill_null = true);

}  // namespace hku