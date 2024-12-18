/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/config.h"
#include "hikyuu/indicator/Indicator.h"

#if HKU_ENABLE_TA_LIB

namespace hku {

/**
 * 求绝对值
 * @ingroup Indicator
 */
Indicator HKU_API TA_CMO(int n = 14);
inline Indicator TA_CMO(const Indicator& ind, int n = 14) {
    return TA_CMO(n)(ind);
}

}  // namespace hku

#endif