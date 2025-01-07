/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ISINF_H_
#define INDICATOR_CRT_ISINF_H_

#include "CVAL.h"

namespace hku {

/**
 * 是否为正无穷大 （负无穷大使用 ISINFA)
 * @ingroup Indicator
 */
Indicator HKU_API ISINF();

inline Indicator ISINF(const Indicator& ind) {
    return ISINF()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ISINF_H_ */
