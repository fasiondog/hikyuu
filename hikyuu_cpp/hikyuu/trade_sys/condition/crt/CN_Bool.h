/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/trade_sys/condition/ConditionBase.h"

namespace hku {

/**
 * System valid condition of the boolean signal
 * @param ind the boolean type indicator; a value > 0 at the corresponding position means the system
 *            is valid, otherwise it is invalid
 * @return
 */
CNPtr HKU_API CN_Bool(const Indicator& ind);

} /* namespace hku */