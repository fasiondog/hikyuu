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
 * @param adjust_cycle 调仓周期，默认为1
 * @param adjust_mode 调仓方式，默认为"query" day|week|month|quarter|year|query
 * @param delay_to_trading_day 延迟至交易日，当调仓日为非交易日时，自动延迟至下一个交易日作为调仓日
 * @ingroup Indicator
 */
Indicator CYCLE(const KData& k, int adjust_cycle = 1, const string& adjust_mode = "query",
                bool delay_to_trading_day = true);
Indicator CYCLE(int adjust_cycle = 1, const string& adjust_mode = "query",
                bool delay_to_trading_day = true);

}  // namespace hku
