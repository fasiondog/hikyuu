/*
 * BACKSET.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BACKSET_H_
#define INDICATOR_CRT_BACKSET_H_

#include "../Indicator.h"

namespace hku {

/**
 * Forward assignment, it sets the data from the current position to several periods before to 1
 * @details
 * <pre>
 * Usage: BACKSET(X,N): if X is not 0, the values from the current position to N periods before are
 * set to 1.
 * For example: BACKSET(CLOSE>OPEN,2) sets the values of the current period and the previous period
 * to 1 if the candle closes up, otherwise 0
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BACKSET(int n = 2);
Indicator HKU_API BACKSET(const IndParam& n);

inline Indicator BACKSET(const Indicator& ind, int n = 2) {
    return BACKSET(n)(ind);
}

inline Indicator BACKSET(const Indicator& ind, const IndParam& n) {
    return BACKSET(n)(ind);
}

inline Indicator BACKSET(const Indicator& ind, const Indicator& n) {
    return BACKSET(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_BACKSET_H_ */
