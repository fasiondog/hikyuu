/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ISNA_H_
#define INDICATOR_CRT_ISNA_H_

#include "../Indicator.h"

namespace hku {

/**
 * Whether it is a NaN value
 * @param ignore_discard whether to ignore the discard values, false by default
 * @ingroup Indicator
 */
Indicator HKU_API ISNA(bool ignore_discard = false);

inline Indicator ISNA(const Indicator& ind, bool ignore_discard = false) {
    return ISNA(ignore_discard)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ISNA_H_ */
