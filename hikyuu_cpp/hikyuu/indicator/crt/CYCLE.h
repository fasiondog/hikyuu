/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

/**
 * PF调仓周期指标，主要用于PF调仓日验证，及作为SG
 * @param k 关联的K线数据
 * @ingroup Indicator
 */
Indicator CYCLE(const KData& k, int adjust_cycle = 1, const string& adjust_mode = "query",
                bool delay_to_trading_day = true);
Indicator CYCLE(int adjust_cycle = 1, const string& adjust_mode = "query",
                bool delay_to_trading_day = true);

}  // namespace hku
