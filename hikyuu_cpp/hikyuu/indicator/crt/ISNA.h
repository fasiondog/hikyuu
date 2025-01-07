/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ISNA_H_
#define INDICATOR_CRT_ISNA_H_

#include "CVAL.h"

namespace hku {

/**
 * 是否为 NaN 值
 * @param ignore_discard 是否忽略discard值，默认为false
 * @ingroup Indicator
 */
Indicator HKU_API ISNA(bool ignore_discard = false);

inline Indicator ISNA(const Indicator& ind, bool ignore_discard = false) {
    return ISNA(ignore_discard)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ISNA_H_ */
