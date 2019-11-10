/*
 * CROSS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_CROSS_H_
#define INDICATOR_CRT_CROSS_H_

#include "CVAL.h"
#include "REF.h"

namespace hku {

/**
 * 两条线交叉, CROSS(A，B)　表示当A从下方向上穿过B时返回1，否则返回0。
 * @ingroup Indicator
 */
Indicator CROSS(const Indicator& x, const Indicator& y);
Indicator CROSS(const Indicator& x, price_t);
Indicator CROSS(price_t, const Indicator& y);
Indicator CROSS(price_t, price_t);

inline Indicator CROSS(const Indicator& x, const Indicator& y) {
    Indicator result = (REF(x, 1) < REF(y, 1)) & (x > y);
    result.name("CROSS");
    return result;
}

inline Indicator CROSS(const Indicator& x, price_t y) {
    return CROSS(x, CVAL(x, y));
}

inline Indicator CROSS(price_t x, const Indicator& y) {
    return CROSS(CVAL(y, x), y);
}

inline Indicator CROSS(price_t x, price_t y) {
    return CROSS(CVAL(x), CVAL(y));
}

}  // namespace hku

#endif /* INDICATOR_CRT_NDAY_H_ */
