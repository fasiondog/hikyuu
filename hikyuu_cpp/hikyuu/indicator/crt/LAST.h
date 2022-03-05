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
 * 区间存在
 * @details
 * <pre>
 * 用法：LAST (X,M,N) 表示条件X在前M周期到前N周期存在
 * 例如：LAST(CLOSE>OPEN,10,5)
 * 表示从前10日到前5日内一直阳线。若A为0,表示从第一天开始,B为0,表示到最后日止。
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
    Indicator ind_n = CVAL(ind_m, 5);
    Indicator max = MAX(ind_m, ind_n);
    Indicator min = MIN(ind_m, ind_n);
    Indicator result = REF(EVERY(max - min + 1), min);
    result.name("LAST");
    return result;
}

inline Indicator LAST(int m, const IndParam& n) {
    Indicator ind_n = n.get();
    Indicator ind_m = CVAL(ind_n, 5);
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
