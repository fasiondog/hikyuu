/*
 * EXP.h
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_EXP_H_
#define INDICATOR_CRT_EXP_H_

#include "../Indicator.h"

namespace hku {

/**
 * 指数, EXP(X)为e的X次幂
 * @ingroup Indicator
 */
Indicator HKU_API EXP();

/**
 * 指数, EXP(X)为e的X次幂
 * @param ind 待计算的数据
 * @ingroup Indicator
 */
Indicator EXP(const Indicator& ind);

inline Indicator EXP(const Indicator& ind) {
    return EXP()(ind);
}

} /* namespace */

#endif /* INDICATOR_CRT_EXP_H_ */
