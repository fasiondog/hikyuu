/*
 * FILTER.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_FILTER_H_
#define INDICATOR_CRT_FILTER_H_

#include "../Indicator.h"

namespace hku {

/**
 * Signal filtering, it filters the signals that appear consecutively.
 * @details
 * <pre>
 * Usage: FILTER(X,N): after X satisfies the condition, the data within the following N periods is
 * deleted and set to 0
 * For example: FILTER(CLOSE>OPEN,5) finds the bullish candles, and the bullish candles appearing
 * again within 5 days are not recorded.
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API FILTER(int n = 5);
Indicator HKU_API FILTER(const IndParam& n);

inline Indicator FILTER(const Indicator& ind, int n = 5) {
    return FILTER(n)(ind);
}

inline Indicator FILTER(const Indicator& ind, const IndParam& n) {
    return FILTER(n)(ind);
}

inline Indicator FILTER(const Indicator& ind, const Indicator& n) {
    return FILTER(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_FILTER_H_ */
