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
 * 对数据进行ZScore标准化，可选进行极值排除
 * @note 非窗口滚动，如需窗口滚动的标准化，直接 (x - MA(x, n)) / STDEV(x, n) 即可
 * @param outExtreme 指示剔除极值
 * @param nsigma 剔除极值时使用的 nsigma 倍 sigma
 * @param recursive 是否进行递归剔除极值
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API ZSCORE(bool outExtreme = false, double nsigma = 3.0, bool recursive = false);

inline Indicator HKU_API ZSCORE(const Indicator& data, bool outExtreme = false, double nsigma = 3.0,
                                bool recursive = false) {
    return ZSCORE(outExtreme, nsigma, recursive)(data);
}

}  // namespace hku