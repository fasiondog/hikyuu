/*
 * SUM.h
 *
 *  Created on: 2019年4月1日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_SUM_H_
#define INDICATOR_CRT_SUM_H_

#include "../Indicator.h"

namespace hku {

/**
 * 求总和。SUM(X,N),统计N周期中X的总和,N=0则从第一个有效值开始。
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator HKU_API SUM(int n = 20);

/**
 * 求总和。SUM(X,N),统计N周期中X的总和,N=0则从第一个有效值开始。
 * @param ind 待计算的数据
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator SUM(const Indicator& ind, int n = 20);

inline Indicator SUM(const Indicator& ind, int n) {
    return SUM(n)(ind);
}

}

#endif /* INDICATOR_CRT_SUM_H_ */
