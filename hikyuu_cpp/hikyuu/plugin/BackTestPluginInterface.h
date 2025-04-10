/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/strategy/Strategy.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class BackTestPluginInterface : public PluginBase {
public:
    BackTestPluginInterface() = default;
    virtual ~BackTestPluginInterface() = default;

    virtual void backtest(const StrategyContext& context,
                          const std::function<void(Strategy*)>& on_bar, const TradeManagerPtr& tm,
                          const Datetime& start_date, const Datetime& end_date,
                          const KQuery::KType& ktype, const string& ref_market, int mode) = 0;
};

}  // namespace hku
