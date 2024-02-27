/*
 * CEILING.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_CEIL_H_
#define INDICATOR_CRT_CEIL_H_

#include "CVAL.h"

namespace hku {

/**
 * 向上舍入 (向数值增大方向舍入)
 * 用法：CEILING(A)返回沿A数值增大方向最接近的整数
 * 例如：CEILING(12.3)求得13；CEILING(-3.5)求得-3
 * @ingroup Indicator
 */
Indicator HKU_API CEILING();
Indicator CEILING(Indicator::value_t);
Indicator CEILING(const Indicator& ind);

inline Indicator CEILING(const Indicator& ind) {
    return CEILING()(ind);
}

inline Indicator CEILING(Indicator::value_t val) {
    return CEILING(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_CEIL_H_ */
