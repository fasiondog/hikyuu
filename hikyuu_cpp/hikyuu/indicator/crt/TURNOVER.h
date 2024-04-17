/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-06
 *      Author: fasiondog
 */

#include "../Indicator.h"

namespace hku {

/**
 * @brief 换手率=股票成交量/流通股股数×100%
 * @param n 窗口周期
 * @return Indicator
 */
Indicator HKU_API TURNOVER(int n = 1);

Indicator HKU_API TURNOVER(const KData& kdata, int n = 1);

}  // namespace hku