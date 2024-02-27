/*
 * ABS.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ABS_H_
#define INDICATOR_CRT_ABS_H_

#include "CVAL.h"

namespace hku {

/**
 * 求绝对值
 * @ingroup Indicator
 */
Indicator HKU_API ABS();
Indicator ABS(Indicator::value_t);
Indicator ABS(const Indicator& ind);

inline Indicator ABS(const Indicator& ind) {
    return ABS()(ind);
}

inline Indicator ABS(Indicator::value_t val) {
    return ABS(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_ABS_H_ */
