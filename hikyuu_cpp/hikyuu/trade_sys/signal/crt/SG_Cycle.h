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
 * It uses the PF rebalancing cycle as the buy signal
 *
 * The days come from the MultiSystem injection (modes A/B) or from the admission declaration
 * (mode C), where an SE that snapshots the buy signals (SE_Signal) never selects it
 *
 * @return SignalPtr
 */
SignalPtr HKU_API SG_Cycle();

}  // namespace hku