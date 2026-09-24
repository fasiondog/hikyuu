/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-06
 *      Author: fasiondog
 */

#pragma once
#include "../Indicator.h"

namespace hku {

/**
 * @brief Turnover rate = stock volume / number of the outstanding shares × 100%
 * @param n window period
 * @return Indicator
 */
Indicator HKU_API TURNOVER(int n = 1);

Indicator HKU_API TURNOVER(const KData& kdata, int n = 1);

}  // namespace hku