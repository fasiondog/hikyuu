/*
 * BARSLASTS.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hikyuu
 */

#pragma once
#ifndef INDICATOR_CRT_BARSLASTS_H_
#define INDICATOR_CRT_BARSLASTS_H_

#include "CVAL.h"

namespace hku {

/**
 * The number of periods from the position where the condition holds for the N-th time to the
 * current one.
 * @details
 * <pre>
 * Usage: BARSLASTS(X, N): the number of days from the N-th time X is not 0 until now.
 * For example: BARSLASTS(CLOSE/REF(CLOSE,1)>=1.1, 2) gives the number of periods from the second
 * limit-up until now.
 * Note: when N=1, BARSLASTS(X, 1) is equivalent to BARSLAST(X).
 * </pre>
 * @param n the N-th time the condition holds, n is a positive integer
 * @ingroup Indicator
 */
Indicator HKU_API BARSLASTS(int n);
Indicator HKU_API BARSLASTS(const IndParam& n);

inline Indicator BARSLASTS(const Indicator& ind, int n) {
    return BARSLASTS(n)(ind);
}

inline Indicator BARSLASTS(const Indicator& ind, const IndParam& n) {
    return BARSLASTS(n)(ind);
}

inline Indicator BARSLASTS(const Indicator& ind, const Indicator& n) {
    return BARSLASTS(IndParam(n))(ind);
}

inline Indicator BARSLASTS(Indicator::value_t val, int n) {
    return BARSLASTS(CVAL(val), n);
}

inline Indicator BARSLASTS(Indicator::value_t val, const IndParam& n) {
    return BARSLASTS(CVAL(val), n);
}

inline Indicator BARSLASTS(Indicator::value_t val, const Indicator& n) {
    return BARSLASTS(CVAL(val), IndParam(n));
}

}  // namespace hku

#endif /* INDICATOR_CRT_BARSLASTS_H_ */
