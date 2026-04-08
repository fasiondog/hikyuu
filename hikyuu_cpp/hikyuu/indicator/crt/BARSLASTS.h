/*
 * BARSLASTS.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hikyuu
 */

#pragma once
#ifndef INDICATOR_CRT_BARSLASTS_H_
#define INDICATOR_CRT_BARSLASTS_H_

#include "CVAL.h"

namespace hku {

/**
 * 第N次条件成立位置到当前的周期数。
 * @details
 * <pre>
 * 用法：BARSLASTS(X, N): 第N次 X 不为 0 到现在的天数。
 * 例如：BARSLASTS(CLOSE/REF(CLOSE,1)>=1.1, 2) 表示第2个涨停板到当前的周期数。
 * 注意：当N=1时，BARSLASTS(X, 1) 等价于 BARSLAST(X)。
 * </pre>
 * @param n 第N次条件成立，n为正整数
 * @ingroup Indicator
 */
Indicator HKU_API BARSLASTS(int n);
Indicator HKU_API BARSLASTS(const IndParam& n);

inline Indicator BARSLASTS(const Indicator& ind, int n) {
    return BARSLASTS(n)(ind);
}

inline Indicator BARSLASTS(const Indicator& ind, const IndParam& n) {
    return BARSLASTS(n)(ind);
}

inline Indicator BARSLASTS(const Indicator& ind, const Indicator& n) {
    return BARSLASTS(IndParam(n))(ind);
}

inline Indicator BARSLASTS(Indicator::value_t val, int n) {
    return BARSLASTS(CVAL(val), n);
}

inline Indicator BARSLASTS(Indicator::value_t val, const IndParam& n) {
    return BARSLASTS(CVAL(val), n);
}

inline Indicator BARSLASTS(Indicator::value_t val, const Indicator& n) {
    return BARSLASTS(CVAL(val), IndParam(n));
}

}  // namespace hku

#endif /* INDICATOR_CRT_BARSLASTS_H_ */
