/*
 * STD.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#ifndef STD_H_
#define STD_H_

#include "../Indicator.h"

namespace hku {

/**
 * 计算N周期内样本标准差
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator HKU_API STDEV(int n = 10);

/**
 * 计算N周期内样本标准差
 * @param data 输入数据，单一输入
 * @param n N日时间窗口
 * @ingroup Indicator
 */
Indicator HKU_API STDEV(const Indicator& data, int n = 10);

} /* namespace */


#endif /* STD_H_ */
