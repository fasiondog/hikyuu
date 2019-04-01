/*
 * ABS.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_ABS_H_
#define INDICATOR_CRT_ABS_H_

#include "../Indicator.h"

namespace hku {

/**
 * 求绝对值
 * @ingroup Indicator
 */
Indicator HKU_API ABS();

/**
 * 求绝对值
 * @param ind 待计算的数据
 * @ingroup Indicator
 */
Indicator ABS(const Indicator& ind);

inline Indicator ABS(const Indicator& ind) {
    return ABS()(ind);
}

} /* namespace */

#endif /* INDICATOR_CRT_ABS_H_ */
