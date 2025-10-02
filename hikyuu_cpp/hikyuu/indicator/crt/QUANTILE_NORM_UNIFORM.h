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
 * 使用分位数法则进行均匀分布标准化（非窗口滚动）
 * @param quantile_min 最小分位数
 * @param quantile_max 最大分位数
 * @return Indicator
 */
Indicator HKU_API QUANTILE_NORM_UNIFORM(double quantile_min = 0.01, double quantile_max = 0.99);

inline Indicator HKU_API QUANTILE_NORM_UNIFORM(const Indicator& data, double quantile_min = 0.01,
                                               double quantile_max = 0.99) {
    return QUANTILE_NORM_UNIFORM(quantile_min, quantile_max)(data);
}

}  // namespace hku