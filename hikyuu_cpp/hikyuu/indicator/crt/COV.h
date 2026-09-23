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
 * Calculate the sample covariance
 * @param ind1 indicator 1
 * @param ind2 indicator 2
 * @param n the rolling window (greater than 2 or equal to 0); when it is 0 the actual length of the
 *          input ind is used.
 * @param fill_null fill the missing dates with nan when the dates are aligned
 * @ingroup Indicator
 */
Indicator HKU_API COV(const Indicator& ind1, const Indicator& ind2, int n = 10,
                      bool fill_null = true);
Indicator HKU_API COV(const Indicator& ref_ind, int n = 10, bool fill_null = true);

}  // namespace hku