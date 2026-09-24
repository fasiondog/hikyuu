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
 * Maximum profit percentage (calculated in the opposite direction corresponding to MDD)
 * @ingroup Indicator
 */
Indicator HKU_API MRR(int n = 0);

/**
 * Maximum profit percentage
 * @param ind the data to be calculated
 * @ingroup Indicator
 */
inline Indicator MRR(const Indicator& ind, int n = 0) {
    return MRR(n)(ind);
}

}  // namespace hku