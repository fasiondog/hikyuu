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
 * @brief Strategy backtest (event driven way)
 *
 * @param context strategy context
 * @param on_bar the main execution function of the strategy
 * @param tm the strategy test account
 * @param start_date start date
 * @param end_date end date (exclusive)
 * @param ktype K-line type
 * @param ref_market the market it belongs to
 * @param mode mode: 0: the buy and sell operations are executed at the current bar close price;
 *             1: they are executed at the next bar open price;
 * @param support_short whether short selling is supported
 * @param slip slippage algorithm
 * @ingroup Strategy
 */
void HKU_API backtest(const StrategyContext& context, const std::function<void(Strategy*)>& on_bar,
                      const TradeManagerPtr& tm, const Datetime& start_date,
                      const Datetime& end_date = Null<Datetime>(),
                      const KQuery::KType& ktype = KQuery::DAY, const string& ref_market = "SH",
                      int mode = 0, bool support_short = false, SlippagePtr slip = SlippagePtr());

// It is used after sm.init only
void HKU_API backtest(const std::function<void(Strategy*)>& on_bar, const TradeManagerPtr& tm,
                      const Datetime& start_date, const Datetime& end_date = Null<Datetime>(),
                      const KQuery::KType& ktype = KQuery::DAY, const string& ref_market = "SH",
                      int mode = 0, bool support_short = false, SlippagePtr slip = SlippagePtr());

}  // namespace hku