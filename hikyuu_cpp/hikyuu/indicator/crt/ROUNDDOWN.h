/*
 * ROUNDDOWN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ROUNDDOWN_H_
#define INDICATOR_CRT_ROUNDDOWN_H_

#include "CVAL.h"

namespace hku {

/**
 * 向上截取，如10.1截取后为11
 * @ingroup Indicator
 */
Indicator HKU_API ROUNDDOWN(int ndigits = 2);
Indicator ROUNDDOWN(Indicator::value_t, int ndigits = 2);
Indicator ROUNDDOWN(const Indicator& ind, int ndigits = 2);

inline Indicator ROUNDDOWN(const Indicator& ind, int n) {
    return ROUNDDOWN(n)(ind);
}

inline Indicator ROUNDDOWN(Indicator::value_t val, int n) {
    return ROUNDDOWN(CVAL(val), n);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROUNDDOWN_H_ */
