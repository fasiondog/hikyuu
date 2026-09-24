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
 * The drawdown percentage from the current point to the historical highest point; it is a positive
 * value as per the industry convention
 * @note the values less than or equal to 0 and the nan values are not handled, nan is returned at
 *       those positions
 * @ingroup Indicator
 */
Indicator HKU_API MDD_CURRENT();

/**
 * The drawdown percentage from the current point to the historical highest point; it is a positive
 * value as per the industry convention
 * @param ind the data to be calculated
 * @ingroup Indicator
 */
inline Indicator MDD_CURRENT(const Indicator& ind) {
    return MDD_CURRENT()(ind);
}

}  // namespace hku