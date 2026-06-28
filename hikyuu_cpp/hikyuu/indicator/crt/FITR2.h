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
 * 计算线性回归拟合优度R²和斜率，N支持变量
 * @param n 时间窗口，需 >= 2
 * @return Indicator，包含两个结果集：
 *         - 结果集0：R²值（拟合优度）
 *         - 结果集1：斜率
 */
Indicator HKU_API FITR2(int n = 22);

/**
 * 计算线性回归拟合优度R²和斜率，N支持变量
 * @param ind 待计算指标
 * @param n 时间窗口，需 >= 2
 * @return Indicator，包含两个结果集：
 *         - 结果集0：R²值（拟合优度）
 *         - 结果集1：斜率
 */
inline Indicator FITR2(const Indicator& ind, int n = 22) {
    return FITR2(n)(ind);
}

}  // namespace hku

#endif