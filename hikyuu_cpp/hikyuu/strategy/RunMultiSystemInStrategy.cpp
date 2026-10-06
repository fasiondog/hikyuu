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

    // The parent account uses the BrokerTM synchronized with the broker; the sub-system accounts
    // are created by MultiSystem::readyForRun by mode
    auto tm = crtBrokerTM(broker, costfunc, ms->name());
    m_ms->setTM(tm);
    m_ms->setSP(SlippagePtr());  // The aggregate parent does not go through the slippage algorithm
    // readyForRun first, THEN the trading objects: readyForRun is what materializes the
    // sub-systems of an SE-only aggregate (it adopts the prototypes held by the SE) and creates the
    // shadow / shared accounts, so refreshing before it would skip exactly those sub-systems and
    // leave them without a trading object until the first scheduled run.
    m_ms->readyForRun();
    _refreshSubKData();
}

void RunMultiSystemInStrategy::_refreshSubKData() {
    _refreshSubKDataRecursive(m_ms);
}

void RunMultiSystemInStrategy::_refreshSubKDataRecursive(const std::shared_ptr<MultiSystem>& ms) {
    HKU_WARN_IF_RETURN(!ms, void(), "Null nested MultiSystem!");
    for (auto& sub : ms->getSystemList()) {
        // The nested aggregate has no instrument of its own, penetrate it to refresh its leaves
        if (sub->isComposite()) {
            _refreshSubKDataRecursive(std::dynamic_pointer_cast<MultiSystem>(sub));
        } else if (!sub->getStock().isNull()) {
            KData kd = sub->getStock().getKData(m_query);
            // An empty KData (the data is not loaded yet, e.g. at construction time) must not
            // overwrite an already valid trading object
            if (!kd.empty()) {
                sub->setTO(kd);
            }
        }
    }
}

void RunMultiSystemInStrategy::run() {
    _refreshSubKData();
    KData k = m_driver_stock.getKData(m_query);
    m_ms->getTM()->fetchAssetInfoFromBroker(m_broker);
    // reset=false: the live daily replay must not clear the sub-system state (the pending delayed
    // requests, the mode B quota, and in mode C the running pool plus the force-sell pool, which is
    // exactly what lets a mode C sub-system keep its admission state from one trading day to the
    // next). The trades of the mode C sub-systems land on the shared BrokerTM, so the broker orders
    // are triggered by that account itself, no extra routing is needed here.
    m_ms->run(k, false);
}

void RunMultiSystemInStrategy::runMomentOnOpen() {
    _refreshSubKData();
    KData k = m_driver_stock.getKData(m_query);
    HKU_WARN_IF_RETURN(k.empty(), void(), "Skip runMomentOnOpen, {} has no loaded data!",
                       m_driver_stock.market_code());
    m_ms->setTO(k);
    m_ms->getTM()->fetchAssetInfoFromBroker(m_broker);
    m_ms->runMomentOnOpen(k.back().datetime);
}

void RunMultiSystemInStrategy::runMomentOnClose() {
    _refreshSubKData();
    KData k = m_driver_stock.getKData(m_query);
    HKU_WARN_IF_RETURN(k.empty(), void(), "Skip runMomentOnClose, {} has no loaded data!",
                       m_driver_stock.market_code());
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
