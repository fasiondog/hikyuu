/*
 * POW.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_POW_H_
#define INDICATOR_CRT_POW_H_

#include "CVAL.h"

namespace hku {

/**
 * Power
 * @details
 * <pre>
 * Usage: POW(A,B) returns A to the power of B
 * For example: POW(CLOSE,3) gives the cube of the close price
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API POW(int n);
Indicator HKU_API POW(const IndParam& n);

inline Indicator POW(const Indicator& ind, int n) {
    return POW(n)(ind);
}

inline Indicator POW(const Indicator& ind, const IndParam& n) {
    return POW(n)(ind);
}

inline Indicator POW(const Indicator& ind, const Indicator& n) {
    return POW(IndParam(n))(ind);
}

inline Indicator POW(Indicator::value_t val, int n) {
    return POW(CVAL(val), n);
}

}  // namespace hku

#endif /* INDICATOR_CRT_POW_H_ */
