/*
 * REF.h
 *
 *  Created on: 2015年3月21日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_REF_H_
#define INDICATOR_CRT_REF_H_

#include "../Indicator.h"

namespace hku {

/**
 * REF 向前引用 （即右移）
 * 引用若干周期前的数据。
 * 用法：　REF(X，A)　引用A周期前的X值。
 * @param n 引用n周期前的值，即右移n位
 * @ingroup Indicator
 */
Indicator HKU_API REF(int n);
Indicator HKU_API REF(const IndParam& n);

/**
 * REF 向前引用 （即右移）
 * 引用若干周期前的数据。
 * 用法：　REF(X，A)　引用A周期前的X值。
 * @param ind 待计算的数据
 * @param n 引用n周期前的值，即右移n位
 * @ingroup Indicator
 */
inline Indicator HKU_API REF(const Indicator& ind, int n) {
    return REF(n)(ind);
}

inline Indicator HKU_API REF(const Indicator& ind, const IndParam& n) {
    return REF(n)(ind);
}

inline Indicator HKU_API REF(const Indicator& ind, const Indicator& n) {
    return REF(IndParam(n))(ind);
}

} /* namespace hku */

#endif /* INDICATOR_CRT_REF_H_ */
