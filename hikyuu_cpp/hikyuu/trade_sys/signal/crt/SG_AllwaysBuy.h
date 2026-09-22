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
 * A system that always issues a buy signal, it is mainly used for the MultiFactor stock selection
 * matching in PF
 * @return SignalPtr
 * @ingroup Signal
 */
SignalPtr HKU_API SG_AllwaysBuy();

}  // namespace hku