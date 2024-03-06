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

namespace hku {

/**
 * 指数, EXP(X)为e的X次幂
 * @ingroup Indicator
 */
Indicator HKU_API EXP();
Indicator EXP(Indicator::value_t);
Indicator EXP(const Indicator& ind);

inline Indicator EXP(const Indicator& ind) {
    return EXP()(ind);
}

inline Indicator EXP(Indicator::value_t val) {
    return EXP(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_EXP_H_ */
