/*
 * SGN.h
 *
 *  Created on: 2019年4月1日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SGN_H_
#define INDICATOR_CRT_SGN_H_

#include "CVAL.h"

namespace hku {

/**
 * 求符号值, SGN(X)，当 X>0, X=0, X<0分别返回 1, 0, -1。
 * @ingroup Indicator
 */
Indicator HKU_API SGN();
Indicator SGN(Indicator::value_t);
Indicator SGN(const Indicator& ind);

inline Indicator SGN(const Indicator& ind) {
    return SGN()(ind);
}

inline Indicator SGN(Indicator::value_t val) {
    return SGN(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_SGN_H_ */
