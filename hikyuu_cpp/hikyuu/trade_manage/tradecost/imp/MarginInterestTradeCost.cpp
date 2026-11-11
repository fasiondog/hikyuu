/*
 * MarginInterestTradeCost.cpp
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#include <cmath>
#include "MarginInterestTradeCost.h"
#include "../crt/TC_Zero.h"
#include "hikyuu/part/PartRegister.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MarginInterestTradeCost)
#endif

namespace hku {

MarginInterestTradeCost::MarginInterestTradeCost() : MarginInterestTradeCost(TC_Zero()) {}

MarginInterestTradeCost::MarginInterestTradeCost(const TradeCostPtr& trade_cost)
: TradeCostBase("TC_MarginInterest") {
    setParam<double>("borrow_cash_rate", 0.0);
    setParam<string>("borrow_stock_rates", "");
    setParam<double>("default_borrow_stock_rate", 0.0);
    m_inner = trade_cost ? trade_cost : TC_Zero();
}

MarginInterestTradeCost::~MarginInterestTradeCost() {}

CostRecord MarginInterestTradeCost::getBuyCost(const Datetime& datetime, const Stock& stock,
                                               price_t price, double num) const {
    return m_inner->getBuyCost(datetime, stock, price, num);
}

CostRecord MarginInterestTradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
                                                price_t price, double num) const {
    return m_inner->getSellCost(datetime, stock, price, num);
}

CostRecord MarginInterestTradeCost::getBorrowCashCost(const Datetime& datetime,
                                                      price_t cash) const {
    return m_inner->getBorrowCashCost(datetime, cash);
}

CostRecord MarginInterestTradeCost::getReturnCashCost(const Datetime& borrow_datetime,
                                                      const Datetime& return_datetime,
                                                      price_t cash) const {
    return m_inner->getReturnCashCost(borrow_datetime, return_datetime, cash);
}

CostRecord MarginInterestTradeCost::getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                                       price_t price, double num) const {
    return m_inner->getBorrowStockCost(datetime, stock, price, num);
}

CostRecord MarginInterestTradeCost::getReturnStockCost(const Datetime& borrow_datetime,
                                                       const Datetime& return_datetime,
                                                       const Stock& stock, price_t price,
                                                       double num) const {
    return m_inner->getReturnStockCost(borrow_datetime, return_datetime, stock, price, num);
}

double MarginInterestTradeCost::getBorrowCashRate(const Datetime& datetime) const {
    return tryGetParam<double>("borrow_cash_rate", 0.0);
}

double MarginInterestTradeCost::getBorrowStockRate(const Datetime& datetime,
                                                   const Stock& stock) const {
    return m_stock_rates.lookup(stock, tryGetParam<double>("default_borrow_stock_rate", 0.0));
}

void MarginInterestTradeCost::_checkParam(const string& name) const {
    if (name == "borrow_stock_rates") {
        // A rate is a decimal annualized ratio of the [0, 1) interval, and 0.0 bills nothing
        m_stock_rates.parse(tryGetParam<string>("borrow_stock_rates", ""),
                            "TC_MarginInterest borrow_stock_rates", true, false);
        return;
    }

    double rate = 0.0;
    if (name == "borrow_cash_rate") {
        rate = tryGetParam<double>("borrow_cash_rate", 0.0);
    } else if (name == "default_borrow_stock_rate") {
        rate = tryGetParam<double>("default_borrow_stock_rate", 0.0);
    } else {
        return;
    }

    HKU_CHECK(std::isfinite(rate) && rate >= 0.0 && rate < 1.0,
              "TC_MarginInterest: {}({}) must be in [0, 1)!", name, rate);
}

TradeCostPtr MarginInterestTradeCost::_clone() {
    // clone copies the params as they are, so the inner model has to be cloned here
    return make_shared<MarginInterestTradeCost>(m_inner ? m_inner->clone() : TC_Zero());
}

TradeCostPtr HKU_API TC_MarginInterest(const TradeCostPtr& trade_cost, double borrow_cash_rate,
                                       const string& borrow_stock_rates,
                                       double default_borrow_stock_rate) {
    auto cost = make_shared<MarginInterestTradeCost>(trade_cost);
    cost->setParam<double>("borrow_cash_rate", borrow_cash_rate);
    cost->setParam<double>("default_borrow_stock_rate", default_borrow_stock_rate);
    cost->setParam<string>("borrow_stock_rates", borrow_stock_rates);
    return cost;
}

void registerPart_TC_MarginInterest() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.tc.margininterest"), void());
    PartMeta meta;
    meta.type = PartType::TC;
    meta.name = "TC_MarginInterest";
    meta.full_name = "core.tc.margininterest";
    meta.params.push_back(
      ParamMeta{.name = "trade_cost", .type = "TradeCost", .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "borrow_cash_rate", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "borrow_stock_rates", .type = "string", .default_value = ""});
    meta.params.push_back(
      ParamMeta{.name = "default_borrow_stock_rate", .type = "double", .default_value = "0.0"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return TC_MarginInterest(
          partOr<TradeCostPtr>(pp, "trade_cost", TC_Zero()), sc.get<double>("borrow_cash_rate"),
          sc.tryGet<string>("borrow_stock_rates", ""), sc.get<double>("default_borrow_stock_rate"));
    });
}

} /* namespace hku */
