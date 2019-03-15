/*
 * SMA.h
 *
 *  Created on: 2015年2月16日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_SMA_H_
#define INDICATOR_CRT_SMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * 简单移动平均
 * @param data 待计算的数据
 * @param n 计算均值的周期窗口，必须为大于0的整数
 * @ingroup Indicator
 */
Indicator HKU_API SMA(const Indicator& data, int n = 22);

Indicator HKU_API SMA(int n = 22);

} /* namespace */



#endif /* INDICATOR_CRT_SMA_H_ */
