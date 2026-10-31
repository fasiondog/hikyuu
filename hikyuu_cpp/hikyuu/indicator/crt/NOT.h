/*
 * NOT.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_NOT_H_
#define INDICATOR_CRT_NOT_H_

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Logical NOT. NOT(X) returns the negation of X, i.e. it returns 1 when X<=0, otherwise 0.
 * @ingroup Indicator
 */
Indicator HKU_API NOT();

/**
 * Logical NOT. NOT(X) returns the negation of X, i.e. it returns 1 when X=0, otherwise 0.
 * @param ind the data to be calculated
 * @ingroup Indicator
 */
inline Indicator NOT(const Indicator& ind) {
    return NOT()(ind);
}

inline void registerPart_NOT() {
    registerIndicatorPart("core.ind.not", [] { return NOT(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_NOT_H_ */
