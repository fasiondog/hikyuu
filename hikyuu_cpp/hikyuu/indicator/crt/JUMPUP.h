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
 * 边缘跳变，从小于等于0.0，跳变到 > 0.0
 * @ingroup Indicator
 */
Indicator HKU_API JUMPUP();

inline Indicator JUMPUP(const Indicator& ind) {
    return JUMPUP()(ind);
}

}  // namespace hku
