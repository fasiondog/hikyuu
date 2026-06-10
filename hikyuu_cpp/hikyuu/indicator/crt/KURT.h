/*
 * KURT.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_KURT_H_
#define INDICATOR_CRT_KURT_H_

#include "../Indicator.h"

namespace hku {

/**
 * 计算N周期内的超额峰度（未调整的总体峰度 - 3）
 * @param n N日时间窗口（大于等于4或等于0），等于0时使用输入的ind实际长度
 * @ingroup Indicator
 */
Indicator HKU_API KURT(int n = 10);
Indicator HKU_API KURT(const IndParam& n);

inline Indicator KURT(const Indicator& data, int n = 10) {
    return KURT(n)(data);
}

inline Indicator KURT(const Indicator& data, const IndParam& n) {
    return KURT(n)(data);
}

inline Indicator KURT(const Indicator& data, const Indicator& n) {
    return KURT(IndParam(n))(data);
}

}  // namespace hku

#endif /* INDICATOR_CRT_KURT_H_ */