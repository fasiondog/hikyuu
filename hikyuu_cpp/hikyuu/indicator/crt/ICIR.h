/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "MA.h"
#include "STDEV.h"

namespace hku {

/**
 * @brief 因子 IR
 * @details IR:信息比率(Information Ratio,简称IR)=
 * IC的多周期均值/IC的标准方差，代表因子获取稳定Alpha的能力。
 * @param ic 已经计算出的 ic 值
 * @param n 时间窗口
 * @return Indicator
 * @ingroup Indicator
 */
inline Indicator ICIR(const Indicator& ic, int n = 10) {
    Indicator x = MA(ic, n) / STDEV(ic, n);
    x.name("IR");
    x.setParam<int>("n", n);
    return x;
}

}  // namespace hku