/*
 * DROPNA.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DROPNA_H_
#define INDICATOR_CRT_DROPNA_H_

#include "../Indicator.h"

namespace hku {

/**
 * Remove the nan values
 * @note A row is removed once any of its result sets is nan, so every result set keeps the same
 * rows and the same length. When drop_inf is true, ±Inf is treated as missing too (rows with any
 * non-finite value are removed).
 * @param drop_inf whether to also drop rows containing ±Inf (default false, keeps the original
 *        NaN-only behavior)
 * @ingroup Indicator
 */
Indicator HKU_API DROPNA(bool drop_inf = false);

inline Indicator DROPNA(const Indicator& ind, bool drop_inf = false) {
    return DROPNA(drop_inf)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_DROPNA_H_ */
