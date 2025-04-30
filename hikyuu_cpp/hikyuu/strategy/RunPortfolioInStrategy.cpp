/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-25
 *      Author: fasiondog
 */

#include <nlohmann/json.hpp>
#include "hikyuu/indicator/crt/CYCLE.h"
#include "BrokerTradeManager.h"
#include "RunPortfolioInStrategy.h"

namespace hku {

RunPortfolioInStrategy::RunPortfolioInStrategy(const PFPtr& pf, const KQuery& query,
                                               const OrderBrokerPtr& broker,
                                               const TradeCostPtr& costfunc)
: m_pf(pf), m_broker(broker) {
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
        m_stocks.insert(sys->getStock());
    }

    auto tm = crtBrokerTM(broker, costfunc, pf->name());
    m_pf->setTM(tm);
}

void RunPortfolioInStrategy::run() {
    // 如果已经有交易记录，不允许同步前移
    if (!m_pf->getTM()->firstDatetime().isNull()) {
        m_pf->getTM()->fetchAssetInfoFromBroker(m_broker);
        m_pf->run(m_query, true);
        return;
    }

    auto brk_asset = m_broker->getAssetInfo();
    if (brk_asset.empty()) {
        HKU_WARN("Failed fetch asset info from broker!");
        return;
    }

    bool need_update_to_adjust_date = true;
    try {
        nlohmann::json asset = nlohmann::json::parse(brk_asset);
        auto& positions = asset["positions"];
        for (auto iter = positions.cbegin(); iter != positions.cend(); ++iter) {
            try {
                const auto& jpos = *iter;
                auto market = jpos["market"].get<string>();
                auto code = jpos["code"].get<string>();
                Stock stock = getStock(fmt::format("{}{}", market, code));
                if (stock.isNull()) {
                    HKU_DEBUG("Not found stock: {}{}", market, code);
                    continue;
                }

                if (m_stocks.find(stock) != m_stocks.end()) {
                    need_update_to_adjust_date = false;
                    break;
                }

            } catch (const std::exception& e) {
                HKU_ERROR(e.what());
            }
        }
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    }

    if (!need_update_to_adjust_date) {
        m_pf->getTM()->fetchAssetInfoFromBroker(m_broker);
        m_pf->run(m_query, true);
        return;
    }

    // 获取最近调仓日
    auto k = getKData("sh000001", m_query);
    auto cycle =
      CYCLE(k, m_pf->getParam<int>("adjust_cycle"), m_pf->getParam<string>("adjust_mode"),
            m_pf->getParam<bool>("delay_to_trading_day"));
    HKU_IF_RETURN(cycle.empty(), void());
    size_t n = cycle.size() - 1 - static_cast<size_t>(cycle[cycle.size() - 1]);
    if (n == 0) {
        m_pf->getTM()->fetchAssetInfoFromBroker(m_broker);
        m_pf->run(m_query, true);
        return;
    }

    Datetime adjust_date = k[n].datetime;
    m_pf->getTM()->fetchAssetInfoFromBroker(m_broker, adjust_date);
    m_pf->run(m_query, true);
}

StrategyPtr HKU_API crtPFStrategy(const PFPtr& pf, const KQuery& query,
                                  const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
                                  const string& name,
                                  const std::vector<OrderBrokerPtr>& other_brokers,
                                  const string& config_file) {
    std::shared_ptr<RunPortfolioInStrategy> runner =
      std::make_shared<RunPortfolioInStrategy>(pf, query, broker, costfunc);

    auto tm = pf->getTM();
    for (const auto& brk : other_brokers) {
        if (brk) {
            tm->regBroker(brk);
        }
    }

    std::function<void(Strategy*)> func = [=](Strategy*) { runner->run(); };

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
    StrategyPtr stg = std::make_shared<Strategy>(code_list, vector<KQuery::KType>{ktype},
                                                 unordered_map<string, int>{}, name, config_file);

    int32_t m = KQuery::getKTypeInMin(ktype);
    if (m < KQuery::getKTypeInMin(KQuery::DAY)) {
        stg->runDaily(std::move(func), Minutes(m));
    } else {
        stg->runDailyAt(std::move(func), TimeDelta(0, 14, 50));
    }
    return stg;
}

}  // namespace hku