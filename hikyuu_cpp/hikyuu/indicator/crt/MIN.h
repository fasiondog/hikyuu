/*
 * MIN.h
 *
 *  Created on: 2019-4-8
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_MIN_H_
#define INDICATOR_CRT_MIN_H_

#include "../Indicator.h"

namespace hku {

#if defined(MIN)
#undef MIN
#endif

/**
 * Calculate the minimum
 * @details
 * <pre>
 * Usage: MIN(A,B) returns the smaller one of A and B
 * For example: MIN(CLOSE,OPEN) returns the smaller one of the open price and the close price
 * </pre>
 * @ingroup Indicator
 */
inline Indicator MIN(const Indicator& ind1, const Indicator& ind2) {
    Indicator result = IF(ind1 < ind2, ind1, ind2);
    result.name("MIN");
    return result;
}

inline Indicator MIN(const Indicator& ind, Indicator::value_t val) {
    Indicator result = IF(ind < val, ind, val);
    result.name("MIN");
    return result;
}

inline Indicator MIN(Indicator::value_t val, const Indicator& ind) {
    Indicator result = IF(val < ind, val, ind);
    result.name("MIN");
    return result;
}

}  // namespace hku

#endif /* INDICATOR_CRT_MIN_H_ */
