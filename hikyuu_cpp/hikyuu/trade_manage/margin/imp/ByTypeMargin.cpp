/*
 * ByTypeMargin.cpp
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#include <cmath>
#include "ByTypeMargin.h"
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/utilities/Log.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ByTypeMargin)
#endif

namespace hku {

ByTypeMargin::ByTypeMargin() : MarginBase("MG_ByType") {
    setParam<string>("rates", "");
    setParam<double>("maintain_rate", 0.0);
    setParam<double>("default_rate", 0.6);
    // The collateral table is separate from the margin one and counts in full when unconfigured
    setParam<string>("collateral_rates", "");
    setParam<double>("default_collateral_rate", 1.0);
}

ByTypeMargin::~ByTypeMargin() {}

void ByTypeMargin::parseRates() const {
    // Both tables take a (0, 1] ratio and reparse only when their own expression changes
    m_rates.parse(getParam<string>("rates"), "MG_ByType rates", false, true);

    // tryGetParam: _checkParam may run before the constructor sets the collateral expression
    m_collateral_rates.parse(tryGetParam<string>("collateral_rates", ""),
                             "MG_ByType collateral_rates", false, true);
}

void ByTypeMargin::_checkParam(const string& name) const {
    double maintain_rate = tryGetParam<double>("maintain_rate", 0.0);
    double default_rate = tryGetParam<double>("default_rate", 0.6);
    double default_collateral_rate = tryGetParam<double>("default_collateral_rate", 1.0);

    HKU_CHECK(std::isfinite(default_rate) && default_rate > 0.0 && default_rate <= 1.0,
              "MG_ByType: default_rate({}) must be in (0, 1]!", default_rate);
    HKU_CHECK(std::isfinite(default_collateral_rate) && default_collateral_rate > 0.0 &&
                default_collateral_rate <= 1.0,
              "MG_ByType: default_collateral_rate({}) must be in (0, 1]!", default_collateral_rate);

    // Parse before validating maintain_rate so a rates change is checked against the new table
    if (name == "rates" || name == "collateral_rates")
        parseRates();

    if (maintain_rate > 0.0) {
        HKU_CHECK(maintain_rate <= 1.0, "MG_ByType: maintain_rate({}) must be <= 1!",
                  maintain_rate);
        // Must stay below every opening ratio, or a position triggers a margin call at once
        double min_open_rate = m_rates.minRate(default_rate);
        HKU_CHECK(maintain_rate <= min_open_rate,
                  "MG_ByType: maintain_rate({}) must be <= the minimum opening rate({})!",
                  maintain_rate, min_open_rate);
    }
}

double ByTypeMargin::rateOf(const Stock& stock) const {
    return m_rates.lookup(stock, getParam<double>("default_rate"));
}

double ByTypeMargin::collateralRateOf(const Stock& stock) const {
    // One table for both directions, matched exactly like the margin one
    return m_collateral_rates.lookup(stock, getParam<double>("default_collateral_rate"));
}

bool ByTypeMargin::isMarginTrading(const Stock& stock) const {
    // An unmatched security is plain cash only when default_rate is 1.0; otherwise it occupies
    // margin at that fallback rate
    if (m_rates.has(stock))
        return true;

    return getParam<double>("default_rate") < 1.0;
}

price_t ByTypeMargin::getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                    double num, MarginSide side) const {
    return rateOf(stock) * nominal(stock, price, num);
}

double ByTypeMargin::getMarginRate(const Datetime& datetime, const Stock& stock,
                                   MarginSide side) const {
    return rateOf(stock);
}

double ByTypeMargin::getCollateralRate(const Datetime& datetime, const Stock& stock,
                                       MarginSide side) const {
    return collateralRateOf(stock);
}

price_t ByTypeMargin::getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                        double num, MarginSide side) const {
    price_t open = getOpenMargin(datetime, stock, price, num, side);
    double maintain_rate = getParam<double>("maintain_rate");
    if (maintain_rate <= 0.0)
        return open;
    return maintain_rate / rateOf(stock) * open;
}

MarginPtr ByTypeMargin::_clone() {
    return make_shared<ByTypeMargin>();
}

MarginPtr HKU_API MG_ByType(const string& rates, double maintain_rate, double default_rate,
                            const string& collateral_rates, double default_collateral_rate) {
    auto margin = make_shared<ByTypeMargin>();
    // Set default_rate first: the maintain_rate check compares against it
    margin->setParam<double>("default_rate", default_rate);
    margin->setParam<double>("maintain_rate", maintain_rate);
    margin->setParam<string>("rates", rates);
    // Same ordering for the collateral fallback and its expression
    margin->setParam<double>("default_collateral_rate", default_collateral_rate);
    margin->setParam<string>("collateral_rates", collateral_rates);
    return margin;
}

void registerPart_MG_ByType() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.mg.by_type"), void());
    PartMeta meta;
    meta.type = PartType::MG;
    meta.name = "MG_ByType";
    meta.full_name = "core.mg.by_type";
    meta.params.push_back(ParamMeta{.name = "rates", .type = "string", .default_value = ""});
    meta.params.push_back(
      ParamMeta{.name = "maintain_rate", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "default_rate", .type = "double", .default_value = "0.6"});
    meta.params.push_back(
      ParamMeta{.name = "collateral_rates", .type = "string", .default_value = ""});
    meta.params.push_back(
      ParamMeta{.name = "default_collateral_rate", .type = "double", .default_value = "1.0"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        // Empty expressions have no textual default, so read them with tryGet
        return MG_ByType(sc.tryGet<string>("rates", ""), sc.get<double>("maintain_rate"),
                         sc.get<double>("default_rate"), sc.tryGet<string>("collateral_rates", ""),
                         sc.tryGet<double>("default_collateral_rate", 1.0));
    });
}

} /* namespace hku */
