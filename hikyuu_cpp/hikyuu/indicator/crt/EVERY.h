/*
 * EVERY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_EVERY_H_
#define INDICATOR_CRT_EVERY_H_

#include "../Indicator.h"

namespace hku {

/**
 * Always existing
 * @details
 * <pre>
 * Usage: EVERY (X,N) means the condition X always exists within N periods
 * For example: EVERY(CLOSE>OPEN,10) means the candles are bullish all the way within the previous
 * 10 days
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API EVERY(int n = 20);
Indicator HKU_API EVERY(const IndParam& n);

inline Indicator EVERY(const Indicator& ind, int n = 20) {
    return EVERY(n)(ind);
}

inline Indicator EVERY(const Indicator& ind, const IndParam& n) {
    return EVERY(n)(ind);
}

inline Indicator EVERY(const Indicator& ind, const Indicator& n) {
    return EVERY(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_EVERY_H_ */
