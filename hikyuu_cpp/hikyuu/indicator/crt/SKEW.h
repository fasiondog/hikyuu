/*
 * SKEW.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SKEW_H_
#define INDICATOR_CRT_SKEW_H_

#include "../Indicator.h"

namespace hku {

/**
 * 计算N周期内未调整的总体偏度
 * @param n N日时间窗口（大于等于3或等于0），等于0时使用输入的ind实际长度
 * @ingroup Indicator
 */
Indicator HKU_API SKEW(int n = 10);
Indicator HKU_API SKEW(const IndParam& n);

inline Indicator SKEW(const Indicator& data, int n = 10) {
    return SKEW(n)(data);
}

inline Indicator SKEW(const Indicator& data, const IndParam& n) {
    return SKEW(n)(data);
}

inline Indicator SKEW(const Indicator& data, const Indicator& n) {
    return SKEW(IndParam(n))(data);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SKEW_H_ */