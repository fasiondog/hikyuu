/*
 * LLV.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2016-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LLV_H_
#define INDICATOR_CRT_LLV_H_

#include "../Indicator.h"

namespace hku {

/**
 * The lowest price within N days
 * @param n N-day time window; when N=0 it starts from the first valid value.
 * @ingroup Indicator
 */
Indicator HKU_API LLV(int n = 20);
Indicator HKU_API LLV(const IndParam& n);

/**
 * The lowest price within N days; when N=0 it starts from the first valid value.
 * @param ind the data to be calculated
 * @param n N-day time window
 * @ingroup Indicator
 */
inline Indicator LLV(const Indicator& ind, int n = 20) {
    return LLV(n)(ind);
}

inline Indicator LLV(const Indicator& ind, const IndParam& n) {
    return LLV(n)(ind);
}

inline Indicator LLV(const Indicator& ind, const Indicator& n) {
    return LLV(IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_LLV_H_ */
