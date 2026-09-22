/*
 * SGN.h
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SGN_H_
#define INDICATOR_CRT_SGN_H_

#include "CVAL.h"

namespace hku {

/**
 * Calculate the sign value, SGN(X) returns 1, 0 and -1 respectively when X>0, X=0 and X<0.
 * @ingroup Indicator
 */
Indicator HKU_API SGN();

inline Indicator SGN(const Indicator& ind) {
    return SGN()(ind);
}

inline Indicator SGN(Indicator::value_t val) {
    return SGN(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_SGN_H_ */
