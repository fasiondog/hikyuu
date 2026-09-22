/*
 * BARSSINCE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BARSSINCE_H_
#define INDICATOR_CRT_BARSSINCE_H_

#include "CVAL.h"

namespace hku {

/**
 * The number of periods from the position where the condition first holds to the current one.

 * @details
 * <pre>
 * Usage: BARSSINCE(X): the number of days from the first time X is not 0 until now.

 * For example: BARSSINCE(HIGH>10) gives the number of periods from the time the stock price exceeds
 * 10 yuan until now

 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSSINCE();

inline Indicator BARSSINCE(const Indicator& ind) {
    return BARSSINCE()(ind);
}

inline Indicator BARSSINCE(Indicator::value_t val) {
    return BARSSINCE(CVAL(val));
}

/**
 * The position where the condition first holds within N periods

 * @details The number of periods from the first time the condition holds within N periods until now

 * <pre>
 * Usage: BARSSINCEN(X,N): the number of periods from the first time X is not 0 within N periods
 * until now, N is a constant BARSSINCEN(X,N):

 * For example: BARSSINCEN(HIGH>10,10) gives the number of periods from the time the stock price
 * exceeds 10 yuan within 10 periods until now

 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSSINCEN(int n);
inline Indicator BARSSINCEN(const Indicator& ind, int n) {
    return BARSSINCEN(n)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_BARSSINCE_H_ */
