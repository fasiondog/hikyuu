/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 *
 *  聚合系统（MultiSystem）实盘入口。
 *  阶段 4：将 MultiSystem 跑在 Strategy 运行时中，父账户使用与券商同步的 BrokerTM，
 *  子系统使用各自的影子/虚拟账户（模式 A/B 由 MultiSystem 内部决定）。
 */

#pragma once
#include "hikyuu/trade_sys/system/imp/MultiSystem.h"
#include "Strategy.h"

namespace hku {

/**
 * @brief 在策略运行时中执行聚合系统 MultiSystem
 * @note 父账户调仓统一在收盘阶段执行。子系统的延迟成交（buy_delay/sell_delay=true 时于下一 bar 开盘兑现）
 *       由 MultiSystem 在开盘阶段缓存、同交易日收盘阶段汇总为对上建议后在父账户下单，故延迟/非延迟子系统均支持。
 *       盘中模式应成对注册 runMomentOnOpen/runMomentOnClose；若仅注册收盘驱动，当日开盘兑现的延迟成交不会被采集
 *       （MultiSystem 内部已做防越界与防跨日残留处理，不会误并前一交易日缓冲）。
 */
class HKU_API RunMultiSystemInStrategy {
public:
    RunMultiSystemInStrategy() = default;

    /**
     * @param ms 聚合交易系统（MultiSystem）
     * @param driver_stock 对齐时间轴的驱动标的（应覆盖各子系统的交易日）
     * @param broker 订单代理（与父账户资产同步的订单代理）
     * @param query 查询条件（起点可指定，终点自动到最新）
     * @param costfunc 成本函数
     */
    RunMultiSystemInStrategy(const std::shared_ptr<MultiSystem>& ms, const Stock& driver_stock,
                             const OrderBrokerPtr& broker, const KQuery& query,
                             const TradeCostPtr& costfunc);
    virtual ~RunMultiSystemInStrategy() = default;

    /** 每日整跑：刷新子系统 KData 至最新后，以驱动标的对齐时间轴重跑整个回测范围 */
    void run();

    /** 盘中模式：开盘阶段驱动一次 */
    void runMomentOnOpen();

    /** 盘中模式：收盘阶段驱动一次 */
    void runMomentOnClose();

private:
    /** 将各子系统 KData 刷新到各自标的的最新范围（实盘每日数据更新） */
    void _refreshSubKData();

private:
    std::shared_ptr<MultiSystem> m_ms;
    Stock m_driver_stock;
    OrderBrokerPtr m_broker;
    KQuery m_query;
};

/**
 * @brief 创建聚合系统策略（MultiSystem 实盘入口）
 * @param ms 聚合交易系统（MultiSystem）
 * @param stk_market_code 驱动标的（如 "SH000001"，作为对齐时间轴）
 * @param query 查询条件
 * @param broker 订单代理
 * @param costfunc 成本函数
 * @param name 策略名称
 * @param other_brokers 其他订单代理（如邮件通知等）
 * @param config_file 配置文件
 */
StrategyPtr HKU_API crtMultiSysStrategy(const std::shared_ptr<MultiSystem>& ms,
                                        const string& stk_market_code, const KQuery& query,
                                        const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
                                        const string& name = "MultiSYSStrategy",
                                        const std::vector<OrderBrokerPtr>& other_brokers = {},
                                        const string& config_file = "");

}  // namespace hku
