/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Edge jump, jumping from greater than 0.0 to <= 0.0
 * @ingroup Indicator
 */
Indicator HKU_API JUMPDOWN();

inline Indicator JUMPDOWN(const Indicator& ind) {
    return JUMPDOWN()(ind);
}

}  // namespace hku
