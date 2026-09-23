/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Perform the data truncation with the quantile rule
 * @param int n window size
 * @param quantile_min minimum quantile
 * @param quantile_max maximum quantile
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API QUANTILE_TRUNC(int n = 60, double quantile_min = 0.01,
                                 double quantile_max = 0.99);

inline Indicator QUANTILE_TRUNC(const Indicator& data, int n = 60, double quantile_min = 0.01,
                                double quantile_max = 0.99) {
    return QUANTILE_TRUNC(n, quantile_min, quantile_max)(data);
}

}  // namespace hku