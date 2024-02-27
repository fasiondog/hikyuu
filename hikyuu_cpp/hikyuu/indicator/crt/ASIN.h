/*
 * ASIN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ASIN_H_
#define INDICATOR_CRT_ASIN_H_

#include "CVAL.h"

namespace hku {

/**
 * 反正弦值
 * @ingroup Indicator
 */
Indicator HKU_API ASIN();
Indicator ASIN(Indicator::value_t);
Indicator ASIN(const Indicator& ind);

inline Indicator ASIN(const Indicator& ind) {
    return ASIN()(ind);
}

inline Indicator ASIN(Indicator::value_t val) {
    return ASIN(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_ASIN_H_ */
