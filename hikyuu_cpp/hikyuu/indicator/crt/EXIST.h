/*
 * EXIST.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_EXIST_H_
#define INDICATOR_CRT_EXIST_H_

#include "../Indicator.h"

namespace hku {

/**
 * Existence
 * @details
 * <pre>
 * Usage: EXIST(X,N) means the condition X exists within N periods
 * For example: EXIST(C>O,10) means there are bullish candles within the previous 10 days
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API EXIST(int n = 20);
Indicator HKU_API EXIST(const IndParam& n);

inline Indicator EXIST(const Indicator& ind, int n = 20) {
    return EXIST(n)(ind);
}

inline Indicator EXIST(const Indicator& ind, const IndParam& n) {
    return EXIST(n)(ind);
}

inline Indicator EXIST(const Indicator& ind, const Indicator& n) {
    return EXIST(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_EXIST_H_ */
