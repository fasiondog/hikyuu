/*
 * DEVSQ.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DEVSQ_H_
#define INDICATOR_CRT_DEVSQ_H_

#include "../Indicator.h"

namespace hku {

/**
 * Sum of squares of the data deviations, it gives the N-day sum of squares of the data deviations
 * of X
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator HKU_API DEVSQ(int n = 10);
Indicator HKU_API DEVSQ(const IndParam& n);

inline Indicator DEVSQ(const Indicator& data, int n = 10) {
    return DEVSQ(n)(data);
}

inline Indicator DEVSQ(const Indicator& data, const IndParam& n) {
    return DEVSQ(n)(data);
}

inline Indicator DEVSQ(const Indicator& data, const Indicator& n) {
    return DEVSQ(IndParam(n))(data);
}

}  // namespace hku

#endif /* INDICATOR_CRT_DEVSQ_H_ */
