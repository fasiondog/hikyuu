/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */

#pragma once
#ifndef WMA_H_
#define WMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * 加权移动平均
 * 算法:Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n)
 * @param n 计算均值的周期窗口，n >= 1
 * @ingroup Indicator
 */
Indicator HKU_API WMA(int n = 22);
// Indicator HKU_API MA(const IndParam& n);

// /**
//  * 简单移动平均
//  * @param ind 待计算的数据
//  * @param n 计算均值的周期窗口，n为0时从第一个有效数据开始计算
//  * @ingroup Indicator
//  */
inline Indicator HKU_API WMA(const Indicator& ind, int n = 22) {
    return WMA(n)(ind);
}

// inline Indicator HKU_API MA(const Indicator& ind, const IndParam& n) {
//     return MA(n)(ind);
// }

// inline Indicator MA(const Indicator& ind, const Indicator& n) {
//     return MA(IndParam(n))(ind);
// }

}  // namespace hku

#endif /* MA_H_ */
