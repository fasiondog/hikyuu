/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 计算 Beta 系数，如衡量资产收益与市场收益之间的敏感性
 * Beta = Cov(stock_return, market_return) / Var(market_return)
 *
 * @note BETA本身不会对输入数据进行收益率转换(pct_change)处理，
 *       输入的指标应为已经计算好的收益率数据。
 * @param ind1 输入指标, 如股票收益率指标
 * @param ind2 对照指标, 如市场收益率指标
 * @param n 滚动窗口 （大于2或等于0），等于0时使用输入的ind实际长度。
 * @param fill_null 日期对齐时，缺失日期填充 nan 值
 * @ingroup Indicator
 */
Indicator HKU_API BETA(const Indicator& ind1, const Indicator& ind2, int n = 10,
                       bool fill_null = true);
Indicator HKU_API BETA(const Indicator& ref_ind, int n = 10, bool fill_null = true);

}  // namespace hku