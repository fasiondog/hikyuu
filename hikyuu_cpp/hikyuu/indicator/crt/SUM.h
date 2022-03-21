/*
 * SUM.h
 *
 *  Created on: 2019年4月1日
 *      Author: fasiondog
 */

#pragma once
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
Indicator HKU_API SUM(const IndParam& n);

/**
 * 求总和。SUM(X,N),统计N周期中X的总和,N=0则从第一个有效值开始。
 * @param ind 待计算的数据
 * @param n N日时间窗口
 * @ingroup Indicator
 */
inline Indicator SUM(const Indicator& ind, int n = 20) {
    return SUM(n)(ind);
}

inline Indicator SUM(const Indicator& ind, const IndParam& n) {
    return SUM(n)(ind);
}

inline Indicator SUM(const Indicator& ind, const Indicator& n) {
    return SUM(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SUM_H_ */
