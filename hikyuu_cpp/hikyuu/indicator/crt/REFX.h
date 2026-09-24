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
 * Enhanced REF (unsafe reference), used to shift the data left or right; when the period is an
 * integer it works the same as REF
 * @note It should not be used for a backtest, it is usually used in scenarios such as the AI model
 *       training
 * @param n reference period
 * @ingroup Indicator
 */
Indicator HKU_API REFX(int n);
inline Indicator REFX(const Indicator& ind, int n) {
    return REFX(n)(ind);
}

}  // namespace hku