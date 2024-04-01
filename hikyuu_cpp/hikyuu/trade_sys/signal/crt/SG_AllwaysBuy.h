/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#pragma once
#include "../SignalBase.h"

namespace hku {

/**
 * 一个总是发出买入信号的系统，主要用于 PF 中 MultiFactor 选股匹配
 * @return SignalPtr
 * @ingroup Signal
 */
SignalPtr HKU_API SG_AllwaysBuy();

}  // namespace hku