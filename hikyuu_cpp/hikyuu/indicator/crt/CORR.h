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
 * 计算样本相关系数与协方差。返回的结果集中，第一个为相关系数，第二个为协方差
 * @param ind1 指标1
 * @param ind2 指标2
 * @param n 滚动窗口 （大于2或等于0），等于0时使用输入的ind实际长度。
 * @param fill_null 日期对齐时，缺失日期填充 nan 值
 * @ingroup Indicator
 */
Indicator HKU_API CORR(const Indicator& ind1, const Indicator& ind2, int n = 10,
                       bool fill_null = true);
Indicator HKU_API CORR(const Indicator& ref_ind, int n = 10, bool fill_null = true);

}  // namespace hku