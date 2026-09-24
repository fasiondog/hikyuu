/*
 * HHV.h
 *
 *  Created on: 2016-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_HHV_H_
#define INDICATOR_CRT_HHV_H_

#include "../Indicator.h"

namespace hku {

/**
 * The highest price within N days; when N=0 it starts from the first valid value
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator HKU_API HHV(int n = 20);
Indicator HKU_API HHV(const IndParam& n);

/**
 * The highest price within N days; when N=0 it starts from the first valid value
 * @param ind the data to be calculated
 * @param n N-day time window
 * @ingroup Indicator
 */
inline Indicator HHV(const Indicator& ind, int n = 20) {
    return HHV(n)(ind);
}

inline Indicator HHV(const Indicator& ind, const IndParam& n) {
    return HHV(n)(ind);
}

inline Indicator HHV(const Indicator& ind, const Indicator& n) {
    return HHV(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_HHV_H_ */
