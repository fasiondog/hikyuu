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
 * 当前价格相对历史最高值的回撤百分比，通常用于计算最大回撤
 * @ingroup Indicator
 */
Indicator HKU_API MDD();

/**
 * 当前价格相对历史最高值的回撤百分比
 * @param ind 待计算的数据
 * @ingroup Indicator
 */
inline Indicator MDD(const Indicator& ind) {
    return MDD()(ind);
}

}  // namespace hku