/*
 * ROUND.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ROUND_H_
#define INDICATOR_CRT_ROUND_H_

#include "CVAL.h"

namespace hku {

/**
 * Round half away from zero (the Chinese traditional 四舍五入) to the given number of decimal
 * places
 * @param ndigits the number of decimal places to keep; a negative value rounds to the left of the
 *                decimal point (e.g. -2 rounds 1234 to 1200)
 * @ingroup Indicator
 */
Indicator HKU_API ROUND(int ndigits = 2);

inline Indicator ROUND(const Indicator& ind, int n = 2) {
    return ROUND(n)(ind);
}

inline Indicator ROUND(Indicator::value_t val, int n = 2) {
    return ROUND(CVAL(val), n);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROUND_H_ */
