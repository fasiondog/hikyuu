/*
 * REF.h
 *
 *  Created on: 2015-3-21
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_REF_H_
#define INDICATOR_CRT_REF_H_

#include "../Indicator.h"

namespace hku {

/**
 * REF forward reference (i.e. shift right)
 * Reference the data of several periods before.
 * Usage: REF(X,A) references the X value A periods before.
 * @param n references the value n periods before, i.e. shifting right by n
 * @ingroup Indicator
 */
Indicator HKU_API REF(int n);
Indicator HKU_API REF(const IndParam& n);

/**
 * REF forward reference (i.e. shift right)
 * Reference the data of several periods before.
 * Usage: REF(X,A) references the X value A periods before.
 * @param ind the data to be calculated
 * @param n references the value n periods before, i.e. shifting right by n
 * @ingroup Indicator
 */
inline Indicator REF(const Indicator& ind, int n) {
    return REF(n)(ind);
}

inline Indicator REF(const Indicator& ind, const IndParam& n) {
    return REF(n)(ind);
}

inline Indicator REF(const Indicator& ind, const Indicator& n) {
    return REF(IndParam(n))(ind);
}

} /* namespace hku */

#endif /* INDICATOR_CRT_REF_H_ */
