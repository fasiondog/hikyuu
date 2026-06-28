/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SLOPE_H_
#define INDICATOR_CRT_SLOPE_H_

#include "../Indicator.h"

namespace hku {

/**
 * 计算线性回归斜率、拟合优度R²和相对最大残差，N支持变量
 * @param n 时间窗口
 * @return Indicator，包含三个结果集：
 *         - result(0): 斜率
 *         - result(1): 拟合优度 R²
 *         - result(2): 相对最大残差 RelMaxRes = max|yi - ŷi| / ȳ
 */
Indicator HKU_API SLOPE(int n = 22);
Indicator HKU_API SLOPE(const IndParam& n);

/**
 * 计算线性回归斜率、拟合优度R²和相对最大残差，N支持变量
 * @param ind 待计算指标
 * @param n 时间窗口
 * @return Indicator，包含三个结果集：
 *         - result(0): 斜率
 *         - result(1): 拟合优度 R²
 *         - result(2): 相对最大残差 RelMaxRes = max|yi - ŷi| / ȳ
 */
inline Indicator SLOPE(const Indicator& ind, int n = 22) {
    return SLOPE(n)(ind);
}

inline Indicator HKU_API SLOPE(const Indicator& ind, const IndParam& n) {
    return SLOPE(n)(ind);
}

inline Indicator SLOPE(const Indicator& ind, const Indicator& n) {
    return SLOPE(IndParam(n))(ind);
}

}  // namespace hku

#endif