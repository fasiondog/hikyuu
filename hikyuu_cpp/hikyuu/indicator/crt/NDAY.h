/*
 * NDAY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_NDAY_H_
#define INDICATOR_CRT_NDAY_H_

#include "EVERY.h"
#include "REF.h"

namespace hku {

/**
 * 连大, NDAY(X,Y,N)表示条件X>Y持续存在N个周期
 * @ingroup Indicator
 */
inline Indicator NDAY(const Indicator& x, const Indicator& y, int n = 3) {
    Indicator result = EVERY(x > y, n);
    result.name("NDAY");
    return result;
}

inline Indicator NDAY(const Indicator& x, const Indicator& y, const Indicator& n) {
    Indicator result = EVERY(x > y, n);
    result.name("NDAY");
    return result;
}

inline Indicator NDAY(const Indicator& x, const Indicator& y, const IndParam& n) {
    Indicator result = EVERY(x > y, n);
    result.name("NDAY");
    return result;
}

}  // namespace hku

#endif /* INDICATOR_CRT_NDAY_H_ */
