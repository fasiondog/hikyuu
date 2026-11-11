/*
 * ByMarketTradeCost.cpp
 *
 *  Created on: 2026-11-12
 *      Author: fasiondog
 */

#include "ByMarketTradeCost.h"
#include <algorithm>
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/utilities/arithmetic.h"
#include "../crt/TC_Zero.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ByMarketTradeCost)
#endif

namespace hku {

ByMarketTradeCost::ByMarketTradeCost() : ByMarketTradeCost(TC_Zero()) {}

ByMarketTradeCost::ByMarketTradeCost(const TradeCostPtr& default_cost)
: TradeCostBase("TC_ByMarket") {
    m_default = default_cost ? default_cost : TC_Zero();
}

ByMarketTradeCost::~ByMarketTradeCost() {}

void ByMarketTradeCost::add(const string& key, const TradeCostPtr& cost) {
    HKU_CHECK(!key.empty(), "The routing key can not be empty!");
    HKU_IF_RETURN(!cost, void());

    string upper_key(key);
    to_upper(upper_key);
    // A "type:<n>" key carries the security type number: a wrong suffix could never match
    if (upper_key.compare(0, 5, "TYPE:") == 0) {
        bool all_digits =
          upper_key.size() > 5 && std::all_of(upper_key.begin() + 5, upper_key.end(),
                                              [](char c) { return c >= '0' && c <= '9'; });
        HKU_CHECK(all_digits, "TC_ByMarket: {} is not a \"type:<n>\" key!", key);
    }
    m_costs[upper_key] = cost;
}

void ByMarketTradeCost::setDefault(const TradeCostPtr& cost) {
    m_default = cost ? cost : TC_Zero();
}

TradeCostPtr ByMarketTradeCost::get(const string& key) const {
    string upper_key(key);
    to_upper(upper_key);
    auto iter = m_costs.find(upper_key);
    return iter == m_costs.end() ? TradeCostPtr() : iter->second;
}

vector<string> ByMarketTradeCost::keys() const {
    vector<string> result;
    result.reserve(m_costs.size());
    for (const auto& item : m_costs) {
        result.push_back(item.first);
    }
    return result;
}

const TradeCostPtr& ByMarketTradeCost::_route(const Stock& stock) const {
    string market(stock.market());
    to_upper(market);
    auto iter = m_costs.find(market);
    if (iter != m_costs.end()) {
        return iter->second;
    }

    string type_key(fmt::format("TYPE:{}", stock.type()));
    iter = m_costs.find(type_key);
    if (iter != m_costs.end()) {
        return iter->second;
    }

    // An unconfigured market is billed by the default model: report it once instead of letting the
    // fallback hide itself in every trade record
    HKU_WARN_IF(m_warned.insert(market).second,
                "TC_ByMarket: no cost model registered for market {} (type {}), the default model "
                "is used!",
                market, stock.type());
    return m_default;
}

CostRecord ByMarketTradeCost::getBuyCost(const Datetime& datetime, const Stock& stock,
                                         price_t price, double num) const {
    return _route(stock)->getBuyCost(datetime, stock, price, num);
}

CostRecord ByMarketTradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const {
    return _route(stock)->getSellCost(datetime, stock, price, num);
}

CostRecord ByMarketTradeCost::getBorrowCashCost(const Datetime& datetime, price_t cash) const {
    return m_default->getBorrowCashCost(datetime, cash);
}

CostRecord ByMarketTradeCost::getReturnCashCost(const Datetime& borrow_datetime,
                                                const Datetime& return_datetime,
                                                price_t cash) const {
    return m_default->getReturnCashCost(borrow_datetime, return_datetime, cash);
}

CostRecord ByMarketTradeCost::getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                                 price_t price, double num) const {
    return _route(stock)->getBorrowStockCost(datetime, stock, price, num);
}

CostRecord ByMarketTradeCost::getReturnStockCost(const Datetime& borrow_datetime,
                                                 const Datetime& return_datetime,
                                                 const Stock& stock, price_t price,
                                                 double num) const {
    return _route(stock)->getReturnStockCost(borrow_datetime, return_datetime, stock, price, num);
}

double ByMarketTradeCost::getBorrowCashRate(const Datetime& datetime) const {
    return m_default->getBorrowCashRate(datetime);
}

double ByMarketTradeCost::getBorrowStockRate(const Datetime& datetime, const Stock& stock) const {
    return _route(stock)->getBorrowStockRate(datetime, stock);
}

bool ByMarketTradeCost::hasPythonPart() const noexcept {
    if (m_is_python_object) {
        return true;
    }
    if (m_default && m_default->hasPythonPart()) {
        return true;
    }
    for (const auto& item : m_costs) {
        if (item.second && item.second->hasPythonPart()) {
            return true;
        }
    }
    return false;
}

TradeCostPtr ByMarketTradeCost::_clone() {
    // clone copies the params, so the routing table has to be rebuilt here
    auto result = make_shared<ByMarketTradeCost>(m_default ? m_default->clone() : TC_Zero());
    for (const auto& item : m_costs) {
        if (item.second) {
            result->m_costs[item.first] = item.second->clone();
        }
    }
    return result;
}

TradeCostPtr HKU_API TC_ByMarket(const TradeCostPtr& default_cost) {
    return make_shared<ByMarketTradeCost>(default_cost);
}

void registerPart_TC_ByMarket() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.tc.bymarket"), void());
    PartMeta meta;
    meta.type = PartType::TC;
    meta.name = "TC_ByMarket";
    meta.full_name = "core.tc.bymarket";
    meta.params.push_back(
      ParamMeta{.name = "default_cost", .type = "TradeCost", .is_part_param = true});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return TC_ByMarket(partOr<TradeCostPtr>(pp, "default_cost", TC_Zero()));
    });
}

} /* namespace hku */
