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
 * Edge jump, jumping from less than or equal to 0.0 to > 0.0
 * @ingroup Indicator
 */
Indicator HKU_API JUMPUP();

inline Indicator JUMPUP(const Indicator& ind) {
    return JUMPUP()(ind);
}

}  // namespace hku
