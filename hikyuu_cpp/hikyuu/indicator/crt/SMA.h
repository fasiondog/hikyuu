/*
 * SMA.h
 *
 *  Created on: 2015年2月16日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SMA_H_
#define INDICATOR_CRT_SMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * 求移动平均
 * @details
 * <pre>
 * 用法：若Y=SMA(X,N,M) 则 Y=[M*X+(N-M)*Y')/N,其中Y'表示上一周期Y值
 * </pre>
 * @param n 计算均值的周期窗口，必须为大于0的整数
 * @param m 系数
 * @ingroup Indicator
 */
Indicator HKU_API SMA(int n = 22, double m = 2.0);
Indicator HKU_API SMA(int, const IndParam& m);
Indicator HKU_API SMA(const IndParam& n, double m = 2.0);
Indicator HKU_API SMA(const IndParam& n, const IndParam& m);

/**
 * 求移动平均
 * @details
 * <pre>
 * 用法：若Y=SMA(X,N,M) 则 Y=[M*X+(N-M)*Y')/N,其中Y'表示上一周期Y值
 * </pre>
 * @param data 待计算的数据
 * @param n 计算均值的周期窗口，必须为大于0的整数
 * @param m 系数
 * @ingroup Indicator
 */
inline Indicator SMA(const Indicator& ind, int n = 22, double m = 2.0) {
    return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, int n, const IndParam& m) {
    return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, const IndParam& n, double m = 2.0) {
    return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, const IndParam& n, const IndParam& m) {
    return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, int n, const Indicator& m) {
    return SMA(n, IndParam(m))(ind);
}

inline Indicator SMA(const Indicator& ind, const Indicator& n, double m = 2.0) {
    return SMA(IndParam(n), m)(ind);
}

inline Indicator SMA(const Indicator& ind, const Indicator& n, const Indicator& m) {
    return SMA(IndParam(n), IndParam(m))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SMA_H_ */
