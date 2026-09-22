/*
 * DMA.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2015-5-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DMA_H_
#define INDICATOR_CRT_DMA_H_

#include "REF.h"

namespace hku {

/**
 * Dynamic moving average

 * @details
 * <pre>
 * Usage: DMA(X,A) gives the dynamic moving average of X.

 * Algorithm: if Y=DMA(X,A) then Y=A*X+(1-A)*Y', where Y' is the Y value of the previous period.

 * For example: DMA(CLOSE,VOL/CAPITAL) gives the average price with the turnover rate as the
 * smoothing factor

 * </pre>
 * @param x the data to be calculated

 * @param a dynamic coefficient

 * @param fill_null fill the missing data with nan when the dates are aligned

 * @ingroup Indicator
 */
Indicator HKU_API DMA(const Indicator& x, const Indicator& a, bool fill_null = true);

}  // namespace hku

#endif /* INDICATOR_CRT_DMA_H_ */
