/*
 * AVEDEV.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2015-5-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_AVEDEV_H_
#define INDICATOR_CRT_AVEDEV_H_

#include "MA.h"
#include "ABS.h"

namespace hku {

/**
 * Mean absolute deviation, it gives the N-day mean absolute deviation of X
 * @param ind the data to be calculated
 * @param n time window
 * @ingroup Indicator
 */
inline Indicator AVEDEV(const Indicator& ind, int n = 22) {
    Indicator result = ABS(ind - MA(ind, n)) / n;
    result.name("AVEDEV");
    return result;
}

inline Indicator AVEDEV(const Indicator& ind, const Indicator& n) {
    Indicator result = ABS(ind - MA(ind, n)) / n;
    result.name("AVEDEV");
    return result;
}

inline Indicator AVEDEV(const Indicator& ind, const IndParam& n) {
    Indicator result = ABS(ind - MA(ind, n)) / n.get();
    result.name("AVEDEV");
    return result;
}

}  // namespace hku

#endif /* INDICATOR_CRT_AVEDEV_H_ */
