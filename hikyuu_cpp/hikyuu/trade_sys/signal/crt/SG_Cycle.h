/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-01
 *      Author: fasiondog
 */

#pragma once

#include "../SignalBase.h"

namespace hku {

/**
 * 以 PF 调仓周期为买入信号
 * @return SignalPtr
 */
SignalPtr HKU_API SG_Cycle();

}  // namespace hku