/*
 * SUMBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SUMBARS_H_
#define INDICATOR_CRT_SUMBARS_H_

#include "CVAL.h"

namespace hku {

/**
 * 累加到指定周期数, 向前累加到指定值到现在的周期数
 * @details
 * <pre>
 * 用法：SUMBARS(X,A):将X向前累加直到大于等于A,返回这个区间的周期数
 * 例如：SUMBARS(VOL,CAPITAL)求完全换手到现在的周期数
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API SUMBARS(double a);
Indicator HKU_API SUMBARS(const IndParam& a);

inline Indicator SUMBARS(const Indicator& ind, double a) {
    return SUMBARS(a)(ind);
}

inline Indicator SUMBARS(const Indicator& ind, const IndParam& a) {
    return SUMBARS(a)(ind);
}

inline Indicator SUMBARS(const Indicator& ind, const Indicator& a) {
    return SUMBARS(IndParam(a))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SUMBARS_H_ */
