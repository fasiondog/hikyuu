/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "../Indicator.h"

namespace hku {

/**
 * Spearman 相关系数
 * @param ind1 指标1
 * @param ind2 指标2
 * @ingroup Indicator
 */
Indicator HKU_API SPEARMAN(const Indicator& ind1, const Indicator& ind2, int n = 0);

}  // namespace hku