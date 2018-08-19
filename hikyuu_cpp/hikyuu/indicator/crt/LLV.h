/*
 * LLV.h
 *
 *  Created on: 2016年4月2日
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_LLV_H_
#define INDICATOR_CRT_LLV_H_

#include "../Indicator.h"

namespace hku {

/**
 * N日内最低价
 * @param n N日时间窗口
 */
Indicator HKU_API LLV(int n = 20);

/**
 * N日内最低价
 * @param ind 待计算的数据
 * @param n N日时间窗口
 */
Indicator HKU_API LLV(const Indicator& ind, int n = 20);

}

#endif /* INDICATOR_CRT_LLV_H_ */
