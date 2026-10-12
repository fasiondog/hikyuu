/*
 * HSL.h
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_HSL_H_
#define INDICATOR_CRT_HSL_H_

#include "KDATA.h"
#include "LIUTONGPAN.h"

namespace hku {

/**
 * Get the turnover rate; multiply it by 100 to get the percentage, it equals
 * VOL(k) / CAPITAL(k) * 0.01
 *
 * @note The transCount unit is normalized by the query ktype: lots (x 100) on the daily line and
 *       above, shares below it; the result is a consistent turnover ratio across all periods
 *
 * @param k the associated K-line data
 * @ingroup Indicator
 */
Indicator HKU_API HSL(const KData& k);
Indicator HKU_API HSL();

}  // namespace hku
#endif /* INDICATOR_CRT_HSL_H_ */
