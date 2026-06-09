/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SIGNED_POWER_H_
#define INDICATOR_CRT_SIGNED_POWER_H_

#include "CVAL.h"

namespace hku {

/**
 * 乘幂
 * @details
 * <pre>
 * SIGNED_POWER(A,B)返回A的B次幂, 但保留原始符号
 * 例如：SIGNED_POWER(CLOSE,3)求得收盘价的3次方，保留原始符号
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API SIGNED_POWER(int n);
Indicator HKU_API SIGNED_POWER(const IndParam& n);

inline Indicator SIGNED_POWER(const Indicator& ind, int n) {
    return SIGNED_POWER(n)(ind);
}

inline Indicator SIGNED_POWER(const Indicator& ind, const IndParam& n) {
    return SIGNED_POWER(n)(ind);
}

inline Indicator SIGNED_POWER(const Indicator& ind, const Indicator& n) {
    return SIGNED_POWER(IndParam(n))(ind);
}

inline Indicator SIGNED_POWER(Indicator::value_t val, int n) {
    return SIGNED_POWER(CVAL(val), n);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SIGNED_POWER_H_ */
