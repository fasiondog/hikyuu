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
 * 差分指标，即data[i] - data[i-1]
 * @ingroup Indicator
 */
Indicator HKU_API DIFF();

/**
 * 差分指标，即data[i] - data[i-1]
 * @param data 待计算数据
 * @ingroup Indicator
 */
Indicator HKU_API DIFF(const Indicator& data);

}  // namespace hku

#endif /* DIFF_H_ */
