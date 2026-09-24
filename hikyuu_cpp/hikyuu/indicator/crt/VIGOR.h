/*
 * VIGOR.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#pragma once
#ifndef VIGOR_H_
#define VIGOR_H_

#include "../Indicator.h"

namespace hku {

/**
 * Alexander Elder's force index

 * @details
 * <pre>
 * See "Come Into My Trading Room" (2007, Earthquake Press) (Alexander Elder) P131

 * Calculation formula: (today's close price - yesterday's close price) * today's volume

 * EMA or MA can generally be used for the smoothing afterwards

 * </pre>
 * @param kdata the K-line data to be calculated

 * @param n EMA smoothing window, it must be greater than or equal to 1

 * @ingroup Indicator
 */
Indicator HKU_API VIGOR(const KData& kdata, int n = 2);

Indicator HKU_API VIGOR(int n = 2);

}  // namespace hku

#endif /* VIGOR_H_ */
