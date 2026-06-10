/*
 * DIFF.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef DIFF_H_
#define DIFF_H_

#include "../Indicator.h"

namespace hku {

/**
 * 差分指标，即data[i] - data[i-n]
 * @param n 差分周期，默认1
 * @ingroup Indicator
 */
Indicator HKU_API DIFF(int n = 1);

/**
 * 差分指标，即data[i] - data[i-n]
 * @param data 待计算数据
 * @param n 差分周期，默认1
 * @ingroup Indicator
 */
Indicator HKU_API DIFF(const Indicator& data, int n = 1);

}  // namespace hku

#endif /* DIFF_H_ */