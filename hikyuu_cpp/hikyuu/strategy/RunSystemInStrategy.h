/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 */

#pragma once
#include "hikyuu/trade_sys/system/System.h"
#include "Strategy.h"

namespace hku {

class HKU_API RunSystemInStrategy {
public:
    RunSystemInStrategy() = default;
    RunSystemInStrategy(const SYSPtr& sys, const OrderBrokerPtr& broker, const KQuery& query,
                        const TradeCostPtr& costfunc);
    virtual ~RunSystemInStrategy() = default;

    void run(const Stock& stock);

private:
    SYSPtr m_sys;
    OrderBrokerPtr m_broker;
    KQuery m_query;

    TradeRequest m_buyRequest;
    TradeRequest m_sellRequest;
};

StrategyPtr HKU_API crtSysStrategy(const SYSPtr& sys, const string& stk_market_code,
                                   const KQuery& query, const OrderBrokerPtr& broker,
                                   const TradeCostPtr& costfunc, const string& name = "SYSStrategy",
                                   const string& config_file = "");

}  // namespace hku