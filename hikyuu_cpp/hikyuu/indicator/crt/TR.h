/*
 * TRG.h
 *
 *  Created on: 2019-3-6

 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TR_H_
#define INDICATOR_CRT_TR_H_

#include "../Indicator.h"

namespace hku {

/**
 * @brief True range (TR)

 * @details
 * <pre>
 * The true range (TR) is the maximum of the following three values:

 *  1. the difference between the high price (H) and the low price (L) of the current period

 *  2. the absolute value of the difference between the high price of the current period and the
 *     close price (PC) of the previous period

 *  3. the absolute value of the difference between the low price of the current period and the close
 *     price of the previous period

 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API TR();
Indicator HKU_API TR(const KData&);

}  // namespace hku

#endif /* INDICATOR_CRT_TR_H_ */
