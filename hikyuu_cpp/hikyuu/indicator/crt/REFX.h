/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-22
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * REF增强（不安全引用）, 用于数据左移或右移，周期为整数时，功能同 REF
 * @note 不应用于回测，通常用于AI模型训练等场景
 * @param n 引用周期
 * @ingroup Indicator
 */
Indicator HKU_API REFX(int n);
inline Indicator HKU_API REFX(const Indicator& ind, int n) {
    return REFX(n)(ind);
}

}  // namespace hku