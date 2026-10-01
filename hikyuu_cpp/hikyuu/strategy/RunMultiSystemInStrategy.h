/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 *
 *  The live trading entry of the aggregate system (MultiSystem).
 *  Run MultiSystem in the Strategy runtime, the parent account uses the BrokerTM synchronized with
 * the broker, the sub-systems use their own shadow/virtual accounts (mode A/B is decided internally
 * by MultiSystem).
 *
 *  Invocation model: RunMultiSystemInStrategy::run() is called exactly ONCE per trading day at a
 * fixed scheduled point (see crtMultiSysStrategy: 14:50 for daily or bar-interval seconds for
 * intraday). The sub-systems replay the historical bars through their own virtual TradeManagers to
 * generate signals
 *  ("backtest"); then at the final point (today's bar), the parent BrokerTM has already been
 * synchronized from the broker snapshot. If the aggregated suggestions indicate a trade, the AF
 * layer uses the parent getFunds(bar_datetime) to revalue the snapshot holdings at the queried
 * bar's market price, corrects the order volume according to the current total assets, and places
 * orders through the broker.
 *
 *  Critical design contract (prevent misread):
 *  - BrokerTM only stores cash + position quantities at the snapshot fetch time; their MARKET VALUE
 * drifts with price as time flows. getFunds(datetime, ktype) MUST revalue the snapshot positions at
 * the queried datetime (cash stays from the snapshot). Do NOT simplify it to always return the
 * no-arg snapshot value (which uses m_datetime / lastDatetime, losing the time-precision). Do NOT
 * change AF callers to no-date getFunds() either -- AF legitimately needs the bar-time valuation to
 * size quotas and target shares.
 *  - There is NO zero-guard for dates earlier than the snapshot: getFunds(date) revalues the
 * snapshot positions at ANY queried datetime using that moment's market price. Do NOT re-introduce
 * a (datetime >= m_datetime) zero guard -- it broke the bar-time valuation, because the daily bar
 * time (00:00) is always earlier than the snapshot time carrying intraday seconds.
 */

#pragma once
#include "hikyuu/trade_sys/system/imp/MultiSystem.h"
#include "Strategy.h"

namespace hku {

/**
 * @brief Execute the aggregate system MultiSystem in the strategy runtime
 * @note The parent account rebalancing is uniformly executed at the close stage. The delayed trades
 * of the sub-systems (fulfilled at the open of the next bar when buy_delay/sell_delay=true) are
 * cached by MultiSystem at the open stage, aggregated into the parent suggestions at the close
 * stage of the same trading day and then ordered on the parent account, so both the delayed and
 * non-delayed sub-systems are supported. In the intraday mode runMomentOnOpen/runMomentOnClose
 * should be registered in pairs; if only the close drive is registered, the delayed trades
 * fulfilled at the open of that day will not be collected (MultiSystem has already done the
 * out-of-bounds and cross-day residue handling internally, it will not wrongly merge the previous
 * trading day's buffer).
 */
class HKU_API RunMultiSystemInStrategy {
public:
    RunMultiSystemInStrategy() = default;

    /**
     * @param ms the aggregate trading system (MultiSystem)
     * @param driver_stock the driving instrument aligning the time axis (it should cover the
     * trading days of every sub-system)
     * @param broker the order broker (the order broker synchronized with the parent account assets)
     * @param query the query condition (the start point can be specified, the end point
     * automatically goes to the latest)
     * @param costfunc the cost function
     */
    RunMultiSystemInStrategy(const std::shared_ptr<MultiSystem>& ms, const Stock& driver_stock,
                             const OrderBrokerPtr& broker, const KQuery& query,
                             const TradeCostPtr& costfunc);
    virtual ~RunMultiSystemInStrategy() = default;

    /** The daily full run: after refreshing the sub-system KData to the latest, rerun the whole
     * backtesting range with the driving instrument aligning the time axis */
    void run();

    /** The intraday mode: drive once at the open stage */
    void runMomentOnOpen();

    /** The intraday mode: drive once at the close stage */
    void runMomentOnClose();

private:
    /** Refresh the KData of every sub-system to the latest range of its own instrument (the daily
     * data update of the live trading) */
    void _refreshSubKData();

private:
    std::shared_ptr<MultiSystem> m_ms;
    Stock m_driver_stock;
    OrderBrokerPtr m_broker;
    KQuery m_query;
};

/**
 * @brief Create the aggregate system strategy (the MultiSystem live trading entry)
 * @param ms the aggregate trading system (MultiSystem)
 * @param stk_market_code the driving instrument (e.g. "SH000001", used to align the time axis)
 * @param query the query condition
 * @param broker the order broker
 * @param costfunc the cost function
 * @param name the strategy name
 * @param other_brokers the other order brokers (e.g. the email notification)
 * @param config_file the config file
 */
StrategyPtr HKU_API crtMultiSysStrategy(const std::shared_ptr<MultiSystem>& ms,
                                        const string& stk_market_code, const KQuery& query,
                                        const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
                                        const string& name = "MultiSYSStrategy",
                                        const std::vector<OrderBrokerPtr>& other_brokers = {},
                                        const string& config_file = "");

}  // namespace hku
