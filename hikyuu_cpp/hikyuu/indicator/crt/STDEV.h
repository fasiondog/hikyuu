/*
 * STD.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef STD_H_
#define STD_H_

#include "../Indicator.h"

namespace hku {

/**
 * Calculate the sample standard deviation within N periods
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator HKU_API STDEV(int n = 10);
Indicator HKU_API STDEV(const IndParam& n);

/**
 * Calculate the sample standard deviation within N periods
 * @param data the input data, a single input
 * @param n N-day time window
 * @ingroup Indicator
 */
inline Indicator STDEV(const Indicator& data, int n = 10) {
    return STDEV(n)(data);
}

inline Indicator STDEV(const Indicator& data, const IndParam& n) {
    return STDEV(n)(data);
}

inline Indicator STDEV(const Indicator& data, const Indicator& n) {
    return STDEV(IndParam(n))(data);
}

}  // namespace hku

#endif /* STD_H_ */
