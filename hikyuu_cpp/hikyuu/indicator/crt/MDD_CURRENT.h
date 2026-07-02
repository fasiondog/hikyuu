/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-02
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 当前点到历史最高点的回撤百分比，按行业惯例为正值
 * @note 不处理小于等于0的值和nan值，这些位置返回nan
 * @ingroup Indicator
 */
Indicator HKU_API MDD_CURRENT();

/**
 * 当前点到历史最高点的回撤百分比，按行业惯例为正值
 * @param ind 待计算的数据
 * @ingroup Indicator
 */
inline Indicator MDD_CURRENT(const Indicator& ind) {
    return MDD_CURRENT()(ind);
}

}  // namespace hku