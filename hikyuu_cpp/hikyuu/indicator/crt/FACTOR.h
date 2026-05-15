/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-15
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"
#include "hikyuu/factor/Factor.h"

namespace hku {

/**
 * 因子指标转换
 * @details 将Factor对象转换为Indicator，使其可以在指标系统中使用。
 *          该指标需要设置K线上下文才能进行计算。
 * @param factor 因子对象
 * @return Indicator对象
 * @ingroup Factor
 */
Indicator HKU_API FACTOR(const Factor& factor);

}  // namespace hku
