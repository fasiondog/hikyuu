/*
 * HHVBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_HHVBARS_H_
#define INDICATOR_CRT_HHVBARS_H_

#include "../Indicator.h"

namespace hku {

/**
 * Position of the previous high; the number of periods from the previous high to the current one.
 * @details
 * <pre>
 * Usage: HHVBARS(X,N): the number of periods from the highest value of X within N periods until
 * now; N=0 means counting starts from the first valid value
 * For example: HHVBARS(HIGH,0) gives the number of periods from the historical new high until now
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API HHVBARS(int n = 20);
Indicator HKU_API HHVBARS(const IndParam& n);

inline Indicator HHVBARS(const Indicator& ind, int n = 20) {
    return HHVBARS(n)(ind);
}

inline Indicator HHVBARS(const Indicator& ind, const IndParam& n) {
    return HHVBARS(n)(ind);
}

inline Indicator HHVBARS(const Indicator& ind, const Indicator& n) {
    return HHVBARS(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_HHVBARS_H_ */
