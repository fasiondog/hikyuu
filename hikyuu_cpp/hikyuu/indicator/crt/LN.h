/*
 * LN.h
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LN_H_
#define INDICATOR_CRT_LN_H_

#include "CVAL.h"

namespace hku {

/**
 * Calculate the natural logarithm
 * Usage: LN(X) is the logarithm with the base e
 * For example: LN(CLOSE) gives the logarithm of the close price
 * @ingroup Indicator
 */
Indicator HKU_API LN();

inline Indicator LN(const Indicator& ind) {
    return LN()(ind);
}

inline Indicator LN(Indicator::value_t val) {
    return LN(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_LN_H_ */
