/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 当前价格相对历史最高值的回撤百分比(n=0 则无时间窗口限制), 按行业惯例为正值
 * @note 不处理小于等0的值和nan值
 * @param n 时间窗口大小
 * @ingroup Indicator
 */
Indicator HKU_API MDD(int n = 0);

/**
 * 当前价格相对历史最高值的回撤百分比
 * @param ind 待计算的数据
 * @param n 时间窗口大小
 * @ingroup Indicator
 */
inline Indicator MDD(const Indicator& ind, int n = 0) {
    return MDD(n)(ind);
}

}  // namespace hku