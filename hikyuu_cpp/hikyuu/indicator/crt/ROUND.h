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
 * 四舍五入
 * @ingroup Indicator
 */
Indicator HKU_API ROUND(int ndigits = 2);
Indicator ROUND(Indicator::value_t, int ndigits = 2);
Indicator ROUND(const Indicator& ind, int ndigits = 2);

inline Indicator ROUND(const Indicator& ind, int n) {
    return ROUND(n)(ind);
}

inline Indicator ROUND(Indicator::value_t val, int n) {
    return ROUND(CVAL(val), n);
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROUND_H_ */
