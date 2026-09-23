/*
 * LAST.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LAST_H_
#define INDICATOR_CRT_LAST_H_

#include "EVERY.h"
#include "REF.h"
#include "MAX.h"
#include "MIN.h"
#include "CVAL.h"

namespace hku {

/**
 * Existence in an interval
 * @details
 * <pre>
 * Usage: LAST (X,M,N) means the condition X exists from M periods ago to N periods ago
 * For example: LAST(CLOSE>OPEN,10,5)
 * means the candles are bullish all the way from 10 days ago to 5 days ago. If A is 0 it means
 * starting from the first day, if B is 0 it means ending at the last day.
 * </pre>
 * @ingroup Indicator
 */
inline Indicator LAST(int m = 10, int n = 5) {
    int max = std::max(m, n);
    int min = std::min(m, n);
    Indicator result = REF(EVERY(max - min + 1), min);
    result.name("LAST");
    return result;
}

inline Indicator LAST(const IndParam& m, int n = 5) {
    Indicator ind_m = m.get();
    Indicator ind_n = CVAL(ind_m, n);
    Indicator max = MAX(ind_m, ind_n);
    Indicator min = MIN(ind_m, ind_n);
    Indicator result = REF(EVERY(max - min + 1), min);
    result.name("LAST");
    return result;
}

inline Indicator LAST(int m, const IndParam& n) {
    Indicator ind_n = n.get();
    Indicator ind_m = CVAL(ind_n, m);
    Indicator max = MAX(ind_m, ind_n);
    Indicator min = MIN(ind_m, ind_n);
    Indicator result = REF(EVERY(max - min + 1), min);
    result.name("LAST");
    return result;
}

inline Indicator LAST(const IndParam& m, const IndParam& n) {
    Indicator ind_m = m.get();
    Indicator ind_n = n.get();
    Indicator max = MAX(ind_m, ind_n);
    Indicator min = MIN(ind_m, ind_n);
    Indicator result = REF(EVERY(max - min + 1), min);
    result.name("LAST");
    return result;
}

inline Indicator LAST(const Indicator& ind, int m = 10, int n = 5) {
    return LAST(m, n)(ind);
}

inline Indicator LAST(const Indicator& ind, const IndParam& m, int n = 5) {
    return LAST(m, n)(ind);
}

inline Indicator LAST(const Indicator& ind, int m, const IndParam& n) {
    return LAST(m, n)(ind);
}

inline Indicator LAST(const Indicator& ind, const IndParam& m, const IndParam& n) {
    return LAST(m, n)(ind);
}

inline Indicator LAST(const Indicator& ind, const Indicator& m, int n = 5) {
    return LAST(IndParam(m), n)(ind);
}

inline Indicator LAST(const Indicator& ind, int m, const Indicator& n) {
    return LAST(m, IndParam(n))(ind);
}

inline Indicator LAST(const Indicator& ind, const Indicator& m, const Indicator& n) {
    return LAST(IndParam(m), IndParam(n))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_LAST_H_ */
