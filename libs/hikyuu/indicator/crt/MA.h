/*
 * MA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#ifndef MA_H_
#define MA_H_

#include "../Indicator.h"

namespace hku {

/**
 * 移动平均
 * @param data 待计算的数据
 * @param n 计算均值的周期窗口，必须为大于0的整数
 * @param type "EMA"|"SMA"|"AMA", 默认"SMA"
 * @ingroup Indicator
 */
Indicator HKU_API MA(const Indicator& data, int n = 22,
        const string& type = "SMA");

Indicator HKU_API MA(int n = 22, const string& type = "SMA");

} /* namespace */


#endif /* MA_H_ */
