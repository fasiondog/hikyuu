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
 * Difference indicator, i.e. data[i] - data[i-n]
 * @param n difference period, 1 by default
 * @ingroup Indicator
 */
Indicator HKU_API DIFF(int n = 1);

/**
 * Difference indicator, i.e. data[i] - data[i-n]
 * @param data the data to be calculated
 * @param n difference period, 1 by default
 * @ingroup Indicator
 */
Indicator HKU_API DIFF(const Indicator& data, int n = 1);

}  // namespace hku

#endif /* DIFF_H_ */