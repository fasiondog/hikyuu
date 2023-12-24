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
 * 当前价格相对历史最低值的盈利百分比
 * @ingroup Indicator
 */
Indicator HKU_API MRR();

/**
 * 当前价格相对历史最低值的盈利百分比
 * @param ind 待计算的数据
 * @ingroup Indicator
 */
inline Indicator MRR(const Indicator& ind) {
    return MRR()(ind);
}

}  // namespace hku