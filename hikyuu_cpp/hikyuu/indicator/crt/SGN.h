/*
 * SUM.h
 *
 *  Created on: 2019年4月1日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_SGN_H_
#define INDICATOR_CRT_SGN_H_

#include "../Indicator.h"

namespace hku {

/**
 * 求符号值, SGN(X)，当 X>0, X=0, X<0分别返回 1, 0, -1。
 * @ingroup Indicator
 */
Indicator HKU_API SGN();

/**
 * 求符号值, SGN(X)，当 X>0, X=0, X<0分别返回 1, 0, -1。
 * @param ind 待计算的数据
 * @ingroup Indicator
 */
Indicator SGN(const Indicator& ind);

inline Indicator SGN(const Indicator& ind) {
    return SGN()(ind);
}

}

#endif /* INDICATOR_CRT_SGN_H_ */
