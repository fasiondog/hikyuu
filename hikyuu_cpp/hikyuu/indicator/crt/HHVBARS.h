/*
 * HHVBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_HHVBARS_H_
#define INDICATOR_CRT_HHVBARS_H_

#include "../Indicator.h"

namespace hku {

/**
 * 上一高点位置 求上一高点到当前的周期数。
 * @details
 * <pre>
 * 用法：HHVBARS(X,N):求N周期内X最高值到当前周期数N=0表示从第一个有效值开始统计
 * 例如：HHVBARS(HIGH,0)求得历史新高到到当前的周期数
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API HHVBARS(int n = 20);
Indicator HKU_API HHVBARS(const IndParam& n);

inline Indicator HHVBARS(const Indicator& ind, int n = 20) {
    return HHVBARS(n)(ind);
}

inline Indicator HHVBARS(const Indicator& ind, const IndParam& n) {
    return HHVBARS(n)(ind);
}

inline Indicator HHVBARS(const Indicator& ind, const Indicator& n) {
    return HHVBARS(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_HHVBARS_H_ */
