/*
 * EXP.h
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_EXP_H_
#define INDICATOR_CRT_EXP_H_

#include "CVAL.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Exponent, EXP(X) is e to the power of X
 * @note Overflow (result beyond the finite range) returns null
 * @ingroup Indicator
 */
Indicator HKU_API EXP();

inline Indicator EXP(const Indicator& ind) {
    return EXP()(ind);
}

inline Indicator EXP(Indicator::value_t val) {
    return EXP(CVAL(val));
}

inline void registerPart_EXP() {
    registerIndicatorPart("core.ind.exp", [] { return EXP(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_EXP_H_ */
