/*
 * FLOOR.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_FLOOR_H_
#define INDICATOR_CRT_FLOOR_H_

#include "CVAL.h"

namespace hku {

/**
 * 向下舍入(向数值减小方向舍入)取整
 * 用法：FLOOR(A)返回沿A数值减小方向最接近的整数
 * 例如：FLOOR(12.3)求得12
 * @ingroup Indicator
 */
Indicator HKU_API FLOOR();
Indicator FLOOR(Indicator::value_t);
Indicator FLOOR(const Indicator& ind);

inline Indicator FLOOR(const Indicator& ind) {
    return FLOOR()(ind);
}

inline Indicator FLOOR(Indicator::value_t val) {
    return FLOOR(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_ROUNDUP_H_ */
