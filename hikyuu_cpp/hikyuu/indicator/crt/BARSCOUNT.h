/*
 * BARSCOUNT.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-12
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BARSCOUNT_H_
#define INDICATOR_CRT_BARSCOUNT_H_

#include "../Indicator.h"

namespace hku {

/**
 * Number of the valid value periods; gives the total number of the periods.
 * @details
 * <pre>
 * Usage: BARSCOUNT(X) gives the number of days from the first valid data until now.
 * For example: for the daily line data BARSCOUNT(CLOSE) gets the total number of the trading days
 *       since the listing, and for the 1-minute line it gets the number of the trading minutes of
 *       the day
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSCOUNT();

inline Indicator BARSCOUNT(const Indicator& ind) {
    return BARSCOUNT()(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_BARSCOUNT_H_ */
