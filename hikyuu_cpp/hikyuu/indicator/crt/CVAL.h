/*
 * CVAL.h
 *
 *  Created on: 2017-6-25
 *      Author: fasiondog
 */

#pragma once
#ifndef CVAL_H_
#define CVAL_H_

#include "../Indicator.h"

namespace hku {

/**
 * 创建一个指定长度的常数指标
 * @param value 常量
 * @param discard 抛弃数量，默认0
 * @ingroup Indicator
 */
Indicator HKU_API CVAL(double value, size_t discard=0);

/**
 * 创建一个常数指标，其长度和输入的ind相同，其值固定为指定value
 * @param ind 待计算的数据
 * @param value 常量
 * @param discard 抛弃数量，默认0
 * @ingroup Indicator
 */
Indicator HKU_API CVAL(const Indicator& ind, double value = 0.0, int discard = 0);

} /* namespace */

#endif /* CVAL_H_ */
