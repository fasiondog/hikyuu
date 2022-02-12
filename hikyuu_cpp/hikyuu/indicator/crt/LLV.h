/*
 * LLV.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2016年4月2日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LLV_H_
#define INDICATOR_CRT_LLV_H_

#include "../Indicator.h"

namespace hku {

/**
 * N日内最低价
 * @param n N日时间窗口, N=0则从第一个有效值开始。
 * @ingroup Indicator
 */
Indicator HKU_API LLV(int n = 20);
Indicator HKU_API LLV(const IndParam& n);

/**
 * N日内最低价, N=0则从第一个有效值开始。
 * @param ind 待计算的数据
 * @param n N日时间窗口
 * @ingroup Indicator
 */
inline Indicator LLV(const Indicator& ind, int n = 20) {
    return LLV(n)(ind);
}

inline Indicator LLV(const Indicator& ind, const IndParam& n) {
    return LLV(n)(ind);
}

inline Indicator LLV(const Indicator& ind, const Indicator& n) {
    return LLV(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_LLV_H_ */
