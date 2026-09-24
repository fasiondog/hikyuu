/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Maximum drawdown percentage (there is no time window limit when n=0); it is a positive value as
 * per the industry convention

 * @note the values less than or equal to 0 and the nan values are not handled

 * @param n time window size

 * @ingroup Indicator
 */
Indicator HKU_API MDD(int n = 0);

/**
 * Maximum drawdown percentage (there is no time window limit when n=0); it is a positive value as
 * per the industry convention

 * @param ind the data to be calculated

 * @param n time window size

 * @ingroup Indicator
 */
inline Indicator MDD(const Indicator& ind, int n = 0) {
    return MDD(n)(ind);
}

}  // namespace hku