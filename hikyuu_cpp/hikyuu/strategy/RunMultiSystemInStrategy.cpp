/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 */

#include "BrokerTradeManager.h"
#include "RunMultiSystemInStrategy.h"

namespace hku {

RunMultiSystemInStrategy::RunMultiSystemInStrategy(const std::shared_ptr<MultiSystem>& ms,
                                                   const Stock& driver_stock,
                                                   const OrderBrokerPtr& broker,
                                                   const KQuery& query,
                                                   const TradeCostPtr& costfunc)
: m_ms(ms), m_driver_stock(driver_stock), m_broker(broker) {
    HKU_ASSERT(ms && broker);

    if (query.queryType() == KQuery::INDEX) {
        m_query = KQueryByIndex(query.start(), Null<int64_t>(), query.kType(), query.recoverType());
    } else if (query.queryType() == KQuery::DATE) {
        m_query =
          KQueryByDate(query.startDatetime(), Null<Datetime>(), query.kType(), query.recoverType());
    } else {
        HKU_THROW("Invalid query: {}", query);
    }

    // 父账户使用与券商同步的 BrokerTM；子系统账户由 MultiSystem::readyForRun 按模式创建
    auto tm = crtBrokerTM(broker, costfunc, ms->name());
    m_ms->setTM(tm);
    m_ms->setSP(SlippagePtr());  // 聚合形态父不经过滑点算法
    m_ms->readyForRun();
}

void RunMultiSystemInStrategy::_refreshSubKData() {
    for (auto& sub : m_ms->getSystemList()) {
        if (!sub->getStock().isNull()) {
            sub->setTO(sub->getStock().getKData(m_query));
        }
    }
}

void RunMultiSystemInStrategy::run() {
    _refreshSubKData();
    KData k = m_driver_stock.getKData(m_query);
    m_ms->getTM()->fetchAssetInfoFromBroker(m_broker);
    m_ms->run(k);
}

void RunMultiSystemInStrategy::runMomentOnOpen() {
    _refreshSubKData();
    KData k = m_driver_stock.getKData(m_query);
    m_ms->setTO(k);
    m_ms->getTM()->fetchAssetInfoFromBroker(m_broker);
    m_ms->runMomentOnOpen(k.back().datetime);
}

void RunMultiSystemInStrategy::runMomentOnClose() {
    _refreshSubKData();
    KData k = m_driver_stock.getKData(m_query);
    m_ms->setTO(k);
    m_ms->getTM()->fetchAssetInfoFromBroker(m_broker);
    m_ms->runMomentOnClose(k.back().datetime);
}

StrategyPtr HKU_API crtMultiSysStrategy(const std::shared_ptr<MultiSystem>& ms,
                                        const string& stk_market_code, const KQuery& query,
                                        const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
                                        const string& name,
                                        const std::vector<OrderBrokerPtr>& other_brokers,
                                        const string& config_file) {
    std::shared_ptr<RunMultiSystemInStrategy> runner = std::make_shared<RunMultiSystemInStrategy>(
      ms, getStock(stk_market_code), broker, query, costfunc);

    auto tm = ms->getTM();
    for (const auto& brk : other_brokers) {
        if (brk) {
            tm->regBroker(brk);
        }
    }

    std::function<void(Strategy*)> func = [=](Strategy*) { runner->run(); };

    KQuery::KType ktype = query.kType();
    StrategyPtr stg = std::make_shared<Strategy>(
      vector<string>{stk_market_code, "SH000001"}, vector<KQuery::KType>{ktype},
      unordered_map<string, int64_t>{}, name, config_file);

    int64_t m = KQuery::getKTypeInSeconds(ktype);
    if (m < KQuery::getKTypeInSeconds(KQuery::DAY)) {
        stg->runDaily(std::move(func), Seconds(m), "SH");
    } else {
        stg->runDailyAt(std::move(func), TimeDelta(0, 14, 50));
    }
    return stg;
}

}  // namespace hku
