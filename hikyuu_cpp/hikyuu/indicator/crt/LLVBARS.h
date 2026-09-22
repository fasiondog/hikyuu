/*
 * LLVBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LLVBARS_H_
#define INDICATOR_CRT_LLVBARS_H_

#include "../Indicator.h"

namespace hku {

/**
 * Position of the previous low; the number of periods from the previous low to the current one.
 * @details
 * <pre>
 * Usage: LLVBARS(X,N): the number of periods from the lowest value of X within N periods until now;
 * N=0 means counting starts from the first valid value
 * For example: LLVBARS(HIGH,20) gives the number of periods from the 20-day lowest point until now
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API LLVBARS(int n = 20);
Indicator HKU_API LLVBARS(const IndParam& n);

inline Indicator LLVBARS(const Indicator& ind, int n = 20) {
    return LLVBARS(n)(ind);
}

inline Indicator LLVBARS(const Indicator& ind, const IndParam& n) {
    return LLVBARS(n)(ind);
}

inline Indicator LLVBARS(const Indicator& ind, const Indicator& n) {
    return LLVBARS(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_LLVBARS_H_ */
