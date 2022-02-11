/*
 * HHV.h
 *
 *  Created on: 2016年4月2日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_HHV_H_
#define INDICATOR_CRT_HHV_H_

#include "../Indicator.h"

namespace hku {

/**
 * N日内最高价, N=0则从第一个有效值开始
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator HKU_API HHV(int n = 20);
Indicator HKU_API HHV(const IndParam& n);

/**
 * N日内最高价, N=0则从第一个有效值开始
 * @param ind 待计算的数据
 * @param n N日时间窗口
 * @ingroup Indicator
 */
inline Indicator HHV(const Indicator& ind, int n = 20) {
    return HHV(n)(ind);
}

inline Indicator HHV(const Indicator& ind, const IndParam& n) {
    return HHV(n)(ind);
}

inline Indicator HHV(const Indicator& ind, const Indicator& n) {
    return HHV(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_HHV_H_ */
