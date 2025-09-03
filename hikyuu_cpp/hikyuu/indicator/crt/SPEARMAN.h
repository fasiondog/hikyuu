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
 * Spearman 相关系数
 * @param ind 指标1
 * @param ref_ind 指标2
 * @param n 滚动窗口(大于2 或 等于0)，等于0时，代表 n 实际使用 ind 的长度
 * @param fill_null 填充缺失值
 * @ingroup Indicator
 */
Indicator HKU_API SPEARMAN(const Indicator& ind, const Indicator& ref_ind, int n = 0,
                           bool fill_null = true);
Indicator HKU_API SPEARMAN(const Indicator& ref_ind, int n = 0, bool fill_null = true);

}  // namespace hku