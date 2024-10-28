/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-25
 *      Author: fasiondog
 */

#include "BrokerTradeManager.h"
#include "RunPortfolioInStrategy.h"

namespace hku {

RunPortfolioInStrategy::RunPortfolioInStrategy(const PFPtr& pf, const KQuery& query,
                                               int adjust_cycle, const OrderBrokerPtr& broker,
                                               const TradeCostPtr& costfunc)
: m_pf(pf), m_broker(broker), m_adjust_cycle(adjust_cycle) {
    HKU_ASSERT(pf && broker);

    if (query.queryType() == KQuery::INDEX) {
        m_query = KQueryByIndex(query.start(), Null<int64_t>(), query.kType(), query.recoverType());
    } else if (query.queryType() == KQuery::DATE) {
        m_query =
          KQueryByDate(query.startDatetime(), Null<Datetime>(), query.kType(), query.recoverType());
    } else {
        HKU_THROW("Invalid query: {}", query);
    }

    auto se = pf->getSE();
    HKU_ASSERT(se);
    const auto& sys_list = se->getProtoSystemList();
    for (const auto& sys : sys_list) {
        HKU_CHECK(!sys->getSP(), "Exist Slippage part in sys, You must clear it! {}", sys->name());
        HKU_CHECK(!sys->getParam<bool>("buy_delay") && !sys->getParam<bool>("sell_delay"),
                  "Thie method only support buy|sell on close!");
    }

    auto tm = crtBrokerTM(broker, costfunc, pf->name());
    m_pf->setTM(tm);
}

void RunPortfolioInStrategy::run() {
    m_pf->getTM()->fetchAssetInfoFromBroker(m_broker);
    m_pf->run(m_query, m_adjust_cycle, true);
}

StrategyPtr HKU_API crtPFStrategy(const PFPtr& pf, const KQuery& query, int adjust_cycle,
                                  const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
                                  const string& name,
                                  const std::vector<OrderBrokerPtr>& other_brokers,
                                  const string& config_file) {
    std::shared_ptr<RunPortfolioInStrategy> runner =
      std::make_shared<RunPortfolioInStrategy>(pf, query, adjust_cycle, broker, costfunc);

    auto tm = pf->getTM();
    for (const auto& brk : other_brokers) {
        if (brk) {
            tm->regBroker(brk);
        }
    }

    std::function<void()> func = [=]() { runner->run(); };

    vector<string> code_list;
    std::set<int64_t> stk_set;
    auto sys_list = pf->getSE()->getProtoSystemList();
    for (const auto& sys : sys_list) {
        auto stock = sys->getStock();
        if (stk_set.count(stock.id()) == 0) {
            stk_set.insert(stock.id());
            code_list.emplace_back(stock.market_code());
        }
    }

    KQuery::KType ktype = query.kType();
    StrategyPtr stg =
      std::make_shared<Strategy>(code_list, vector<KQuery::KType>{ktype}, name, config_file);

    int32_t m = KQuery::getKTypeInMin(ktype);
    if (m < KQuery::getKTypeInMin(KQuery::DAY)) {
        stg->runDaily(std::move(func), Minutes(m));
    } else {
        stg->runDailyAt(std::move(func), TimeDelta(0, 14, 50));
    }
    return stg;
}

}  // namespace hku