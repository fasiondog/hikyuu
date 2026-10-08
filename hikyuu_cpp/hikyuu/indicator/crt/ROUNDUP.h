/*
 * ROUNDUP.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ROUNDUP_H_
#define INDICATOR_CRT_ROUNDUP_H_

#include "CVAL.h"

namespace hku {

/**
 * Round away from zero, e.g. 10.1 rounds to 11 and -10.1 rounds to -11
 * @param ndigits the number of decimal places to keep; a negative value rounds to the left of the
 *                decimal point
 * @ingroup Indicator
 */
Indicator HKU_API ROUNDUP(int ndigits = 2);

inline Indicator ROUNDUP(const Indicator& ind, int n = 2) {
    return ROUNDUP(n)(ind);
}

inline Indicator ROUNDUP(Indicator::value_t val, int n = 2) {
    return ROUNDUP(CVAL(val), n);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROUNDUP_H_ */
