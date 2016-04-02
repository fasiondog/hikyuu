/*
 * EMA.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#ifndef EMA_H_
#define EMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * 指数移动平均线(Exponential Moving Average)
 * @param n 计算均值的周期窗口，必须为大于0的整数
 * @ingroup Indicator
 */
Indicator HKU_API EMA(int n = 22);

/**
 * 指数移动平均线(Exponential Moving Average)
 * @param data 待计算的源数据
 * @param n 计算均值的周期窗口，必须为大于0的整数
 * @ingroup Indicator
 */
Indicator HKU_API EMA(const Indicator& data, int n = 22);

} /* namespace */

#endif /* EMA_H_ */
