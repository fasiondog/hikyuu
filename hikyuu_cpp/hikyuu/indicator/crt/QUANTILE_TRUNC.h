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
 * 使用分位数法则进行数据截断处理
 * @param int n 窗口大小
 * @param quantile_min 最小分位数
 * @param quantile_max 最大分位数
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API QUANTILE_TRUNC(int n = 60, double quantile_min = 0.01,
                                 double quantile_max = 0.99);

inline Indicator HKU_API QUANTILE_TRUNC(const Indicator& data, int n = 60,
                                        double quantile_min = 0.01, double quantile_max = 0.99) {
    return QUANTILE_TRUNC(n, quantile_min, quantile_max)(data);
}

}  // namespace hku