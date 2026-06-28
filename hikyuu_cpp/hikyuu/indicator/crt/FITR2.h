/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_FITR2_H_
#define INDICATOR_CRT_FITR2_H_

#include "../Indicator.h"

namespace hku {

/**
 * 计算线性回归拟合优度R²，N支持变量
 * @param n 时间窗口，需 >= 2
 * @return Indicator
 */
Indicator HKU_API FITR2(int n = 22);

/**
 * 计算线性回归拟合优度R²，N支持变量
 * @param ind 待计算指标
 * @param n 时间窗口，需 >= 2
 * @return Indicator
 */
inline Indicator FITR2(const Indicator& ind, int n = 22) {
    return FITR2(n)(ind);
}

}  // namespace hku

#endif