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
 * Spearman correlation coefficient
 * @param ind indicator 1
 * @param ref_ind indicator 2
 * @param n the rolling window (greater than 2 or equal to 0); when it is 0, n actually uses the
 *          length of ind
 * @param fill_null fill the missing values
 * @ingroup Indicator
 */
Indicator HKU_API SPEARMAN(const Indicator& ind, const Indicator& ref_ind, int n = 0,
                           bool fill_null = true);
Indicator HKU_API SPEARMAN(const Indicator& ref_ind, int n = 0, bool fill_null = true);

}  // namespace hku