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
 * 向上截取，如10.1截取后为11
 * @ingroup Indicator
 */
Indicator HKU_API ROUNDUP(int ndigits = 2);
Indicator ROUNDUP(Indicator::value_t, int ndigits = 2);
Indicator ROUNDUP(const Indicator& ind, int ndigits = 2);

inline Indicator ROUNDUP(const Indicator& ind, int n) {
    return ROUNDUP(n)(ind);
}

inline Indicator ROUNDUP(Indicator::value_t val, int n) {
    return ROUNDUP(CVAL(val), n);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROUNDUP_H_ */
