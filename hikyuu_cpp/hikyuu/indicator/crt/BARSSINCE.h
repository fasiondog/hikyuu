/*
 * BARSSINCE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BARSSINCE_H_
#define INDICATOR_CRT_BARSSINCE_H_

#include "CVAL.h"

namespace hku {

/**
 * 第一个条件成立位置到当前的周期数。
 * @details
 * <pre>
 * 用法：BARSSINCE(X):第一次X不为0到现在的天数。
 * 例如：BARSSINCE(HIGH>10)表示股价超过10元时到当前的周期数
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSSINCE();

inline Indicator BARSSINCE(const Indicator& ind) {
    return BARSSINCE()(ind);
}

inline Indicator BARSSINCE(Indicator::value_t val) {
    return BARSSINCE(CVAL(val));
}

/**
 * N周期内首个条件成立位置
 * @details N周期内第一个条件成立到当前的周期数
 * <pre>
 * 用法：BARSSINCEN(X,N):N周期内第一次X不为0到现在的周期数,N为常量BARSSINCEN(X,N):
 * 例如：BARSSINCEN(HIGH>10,10)表示10个周期内股价超过10元时到当前的周期数
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSSINCEN(int n);
inline Indicator BARSSINCEN(const Indicator& ind, int n) {
    return BARSSINCEN(n)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_BARSSINCE_H_ */
