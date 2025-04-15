/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#pragma once
#include "hikyuu/strategy/Strategy.h"

namespace hku {

/**
 * @brief 策略回测（事件驱动方式）
 *
 * @param context 策略上下文
 * @param on_bar 策略主体执行函数
 * @param tm 策略测试账户
 * @param start_date 起始日期
 * @param end_date 结束日期（不包含本身）
 * @param ktype K线类型
 * @param ref_market 所属市场
 * @param mode 模式  0: 当前bar收盘价执行买卖操作；1: 下一bar开盘价执行买卖操作;
 */
void HKU_API backtest(const StrategyContext& context, const std::function<void(Strategy*)>& on_bar,
                      const TradeManagerPtr& tm, const Datetime& start_date,
                      const Datetime& end_date = Null<Datetime>(),
                      const KQuery::KType& ktype = KQuery::DAY, const string& ref_market = "SH",
                      int mode = 0);

// 仅在 sm.init 之后使用
void HKU_API backtest(const std::function<void(Strategy*)>& on_bar, const TradeManagerPtr& tm,
                      const Datetime& start_date, const Datetime& end_date = Null<Datetime>(),
                      const KQuery::KType& ktype = KQuery::DAY, const string& ref_market = "SH",
                      int mode = 0);

}  // namespace hku