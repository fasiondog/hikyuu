/*
 * INTPART.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_INTPART_H_
#define INDICATOR_CRT_INTPART_H_

#include "CVAL.h"

namespace hku {

/**
 * 向上舍入 (向数值增大方向舍入)
 * 用法：CEILING(A)返回沿A数值增大方向最接近的整数
 * 例如：CEILING(12.3)求得13；CEILING(-3.5)求得-3
 * @ingroup Indicator
 */
Indicator HKU_API INTPART();
Indicator INTPART(Indicator::value_t);
Indicator INTPART(const Indicator& ind);

inline Indicator INTPART(const Indicator& ind) {
    return INTPART()(ind);
}

inline Indicator INTPART(Indicator::value_t val) {
    return INTPART(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_INTPART_H_ */
