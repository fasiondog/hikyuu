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
 * @param ind1 指标1
 * @param ind2 指标2
 * @param n 滚动窗口(大于2 或 等于0)，等于0时，代表 n 实际使用 ind 的长度
 * @ingroup Indicator
 */
Indicator HKU_API SPEARMAN(int n = 0);
Indicator HKU_API SPEARMAN(const Indicator& ind1, const Indicator& ind2, int n = 0);

}  // namespace hku