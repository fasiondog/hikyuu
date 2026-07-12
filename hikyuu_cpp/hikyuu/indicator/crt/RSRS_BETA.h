/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 原始 RSRS（底层 β）指标，基于滚动N日OLS回归
 * 公式：High = α + β · Low
 *
 * 每根K线贡献一个坐标点 (Low[i], High[i])，使用滚动窗口内的N个点进行OLS回归。
 * β 为最原始的 RSRS 斜率，代表支撑阻力强弱。
 * 缺陷：不同行情区间 β 中枢波动大，不能跨时段直接对比。
 *
 * @param n 滚动窗口，默认为20
 * @param kdata K线数据
 * @ingroup Indicator
 */
Indicator HKU_API RSRS_BETA(int n = 20);
Indicator HKU_API RSRS_BETA(const KData& kdata, int n = 20);

}  // namespace hku