/*
 * SQRT.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SQRT_H_
#define INDICATOR_CRT_SQRT_H_

#include "CVAL.h"

namespace hku {

/**
 * 开平方
 * @details
 * <pre>
 * 用法：SQRT(X)为X的平方根
 * 例如：SQRT(CLOSE)收盘价的平方根
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API SQRT();
Indicator SQRT(Indicator::value_t);
Indicator SQRT(const Indicator& ind);

inline Indicator SQRT(const Indicator& ind) {
    return SQRT()(ind);
}

inline Indicator SQRT(Indicator::value_t val) {
    return SQRT(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_SQRT_H_ */
