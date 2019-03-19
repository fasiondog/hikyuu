/*
 * HHV.h
 *
 *  Created on: 2016年4月2日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_HHV_H_
#define INDICATOR_CRT_HHV_H_

#include "../Indicator.h"

namespace hku {

/**
 * N日内最高价
 * @param n N日时间窗口
 */
Indicator HKU_API HHV(int n = 20);

/**
 * N日内最高价
 * @param ind 待计算的数据
 * @param n N日时间窗口
 */
Indicator HKU_API HHV(const Indicator& ind, int n = 20);

}

#endif /* INDICATOR_CRT_HHV_H_ */
