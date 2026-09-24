/*
 * NOT.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_NOT_H_
#define INDICATOR_CRT_NOT_H_

#include "../Indicator.h"

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

}  // namespace hku

#endif /* INDICATOR_CRT_NOT_H_ */
