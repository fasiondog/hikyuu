/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 */

#include "BrokerTradeManager.h"
#include "RunSystemInStrategy.h"

namespace hku {

RunSystemInStrategy::RunSystemInStrategy(const SYSPtr& sys, const OrderBrokerPtr& broker,
                                         const KQuery& query, const TradeCostPtr& costfunc)
: m_sys(sys), m_broker(broker) {
    HKU_ASSERT(sys && broker);

    if (query.queryType() == KQuery::INDEX) {
        m_query = KQueryByIndex(query.start(), Null<int64_t>(), query.kType(), query.recoverType());
    } else if (query.queryType() == KQuery::DATE) {
        m_query =
          KQueryByDate(query.startDatetime(), Null<Datetime>(), query.kType(), query.recoverType());
    } else {
        HKU_THROW("Invalid query: {}", query);
    }

    auto tm = crtBrokerTM(broker, costfunc, sys->name());
    m_sys->setTM(tm);
    m_sys->setSP(SlippagePtr());  // 清除移滑价差算法
}

void RunSystemInStrategy::run(const Stock& stock) {
    if (m_sys->getParam<bool>("buy_delay") && m_buyRequest.valid) {
        KData k = stock.getKData(
          KQueryByIndex(-1, Null<int64_t>(), m_query.kType(), m_query.recoverType()));
        const auto& stock = m_sys->getStock();
        m_broker->buy(m_buyRequest.datetime, stock.market(), stock.code(), 10.0,
                      m_buyRequest.number, m_buyRequest.stoploss, m_buyRequest.goal,
                      m_buyRequest.from);
    }

    if (m_sys->getParam<bool>("sell_delay") && m_sellRequest.valid) {
        KData k = stock.getKData(
          KQueryByIndex(-1, Null<int64_t>(), m_query.kType(), m_query.recoverType()));
        const auto& stock = m_sys->getStock();
        m_broker->sell(m_sellRequest.datetime, stock.market(), stock.code(), 10.0,
                       m_sellRequest.number, m_sellRequest.stoploss, m_sellRequest.goal,
                       m_sellRequest.from);
    }

    m_sys->getTM()->fetchAssetInfoFromBroker(m_broker);
    m_sys->run(stock, m_query);

    if (m_sys->getParam<bool>("buy_delay")) {
        m_buyRequest = m_sys->getBuyTradeRequest();
    }

    if (m_sys->getParam<bool>("sell_delay")) {
        m_sellRequest = m_sys->getSellTradeRequest();
    }
}

StrategyPtr HKU_API crtSysStrategy(const SYSPtr& sys, const string& stk_market_code,
                                   const KQuery& query, const OrderBrokerPtr& broker,
                                   const TradeCostPtr& costfunc, const string& name,
                                   const std::vector<OrderBrokerPtr>& other_brokers,
                                   const string& config_file) {
    std::shared_ptr<RunSystemInStrategy> runner =
      std::make_shared<RunSystemInStrategy>(sys, broker, query, costfunc);

    auto tm = sys->getTM();
    for (const auto& brk : other_brokers) {
        if (brk) {
            tm->regBroker(brk);
        }
    }

    std::function<void(const Strategy&)> func = [=](const Strategy&) {
        runner->run(getStock(stk_market_code));
    };

    KQuery::KType ktype = query.kType();
    StrategyPtr stg = std::make_shared<Strategy>(vector<string>{stk_market_code, "SH000001"},
                                                 vector<KQuery::KType>{ktype},
                                                 unordered_map<string, int>{}, name, config_file);

    int32_t m = KQuery::getKTypeInMin(ktype);
    if (m < KQuery::getKTypeInMin(KQuery::DAY)) {
        stg->runDaily(std::move(func), Minutes(m), "SH");
    } else {
        stg->runDailyAt(std::move(func), TimeDelta(0, 14, 50));
    }
    return stg;
}

}  // namespace hku