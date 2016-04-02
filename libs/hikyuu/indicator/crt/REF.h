/*
 * REF.h
 *
 *  Created on: 2015年3月21日
 *      Author: Administrator
 */

#ifndef INDICATOR_CRT_REF_H_
#define INDICATOR_CRT_REF_H_

#include "../Indicator.h"

namespace hku {

/**
 * REF 向前引用 （即右移）
 * 引用若干周期前的数据。
 * 用法：　REF(X，A)　引用A周期前的X值。
 * 例如：　REF(CLOSE，1)　表示上一周期的收盘价，在日线上就是昨收。
 * @param n 引用n周期前的值，即右移n位
 * @ingroup Indicator
 */
Indicator HKU_API REF(int n);

/**
 * REF 向前引用 （即右移）
 * 引用若干周期前的数据。
 * 用法：　REF(X，A)　引用A周期前的X值。
 * 例如：　REF(CLOSE，1)　表示上一周期的收盘价，在日线上就是昨收。
 * @param ind 待计算的数据
 * @param n 引用n周期前的值，即右移n位
 * @ingroup Indicator
 */
Indicator HKU_API REF(const Indicator& ind, int n);

} /* namespace hku */

#endif /* INDICATOR_CRT_REF_H_ */
