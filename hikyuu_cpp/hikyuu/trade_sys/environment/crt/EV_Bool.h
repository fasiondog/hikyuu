/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-03
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/trade_sys/environment/EnvironmentBase.h"

namespace hku {

/**
 * Market environment of the boolean signal
 * @param ind the boolean type indicator; a value > 0 at the corresponding position means the market
 *            is valid, otherwise it is invalid
 * @param market the given market, used to get the corresponding trading calendar
 * @return
 */
EVPtr HKU_API EV_Bool(const Indicator& ind, const string& market = "SH");

}  // namespace hku