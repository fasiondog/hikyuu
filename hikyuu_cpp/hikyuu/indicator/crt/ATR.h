/*
 * ATR.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */

#pragma once
#ifndef INDICATOR_CRT_ATR_H_
#define INDICATOR_CRT_ATR_H_

#include "../Indicator.h"

namespace hku {

/**
 * Average True Range (ATR), the simple average of TR
 * @param n the period window for calculating the average, it must be an integer greater than 1
 * @ingroup Indicator
 */
Indicator HKU_API ATR(int n = 14);

/**
 * Average True Range (ATR)
 * @param kdata the source data to be calculated
 * @param n the period window for calculating the average, it must be an integer greater than 1
 * @ingroup Indicator
 */
Indicator HKU_API ATR(const KData& kdata, int n = 14);

}  // namespace hku

#endif /* INDICATOR_CRT_ATR_H_ */
