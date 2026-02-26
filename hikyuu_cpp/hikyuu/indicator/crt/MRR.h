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
 * 最大盈利百分比(和MDD向对应的相反方向计算)
 * @ingroup Indicator
 */
Indicator HKU_API MRR(int n = 0);

/**
 * 最大盈利百分比
 * @param ind 待计算的数据
 * @ingroup Indicator
 */
inline Indicator MRR(const Indicator& ind, int n = 0) {
    return MRR(n)(ind);
}

}  // namespace hku