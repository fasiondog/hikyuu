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
 * PF rebalancing cycle indicator, mainly used for verifying the PF rebalancing days and as SG
 * @param k the associated K-line data
 * @param adjust_cycle rebalancing cycle, 1 by default
 * @param adjust_mode rebalancing mode, "query" by default: day|week|month|quarter|year|query
 * @param delay_to_trading_day delay to the trading day; when the rebalancing day is a non-trading
 *                             day, it is automatically delayed to the next trading day as the
 *                             rebalancing day
 * @ingroup Indicator
 */
Indicator HKU_API CYCLE(const KData& k, int adjust_cycle = 1, const string& adjust_mode = "query",
                        bool delay_to_trading_day = true);
Indicator HKU_API CYCLE(int adjust_cycle = 1, const string& adjust_mode = "query",
                        bool delay_to_trading_day = true);

}  // namespace hku
