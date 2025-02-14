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
 * 边缘跳变，从大于0.0，跳变到 <= 0.0
 * @ingroup Indicator
 */
Indicator HKU_API JUMPDOWN();

inline Indicator JUMPDOWN(const Indicator& ind) {
    return JUMPDOWN()(ind);
}

}  // namespace hku
