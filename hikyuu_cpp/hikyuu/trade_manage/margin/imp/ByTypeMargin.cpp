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
    // Both tables take a ratio of the (0, 1] interval, and each reparses only when its own
    // expression changed
    m_rates.parse(getParam<string>("rates"), "MG_ByType rates", false, true);

    // tryGetParam: _checkParam runs while the constructor is still setting the params, so the
    // collateral expression may not exist yet
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

    // Parse before validating the maintain ratio, so a rates change re-validates it against the
    // new table (each table is reparsed only when its own expression changed)
    if (name == "rates" || name == "collateral_rates")
        parseRates();

    if (maintain_rate > 0.0) {
        HKU_CHECK(maintain_rate <= 1.0, "MG_ByType: maintain_rate({}) must be <= 1!",
                  maintain_rate);
        // The maintenance margin must stay below every opening ratio, otherwise the account would
        // be called the moment a position is opened
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
    // A position of a security matched by no key is a plain cash position only when the fallback
    // ratio is 1.0, otherwise it occupies the margin of that fallback ratio
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
    // default_rate is set first because the maintain_rate check compares against it
    margin->setParam<double>("default_rate", default_rate);
    margin->setParam<double>("maintain_rate", maintain_rate);
    margin->setParam<string>("rates", rates);
    // the fallback rate is set before the expression for the same reason
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
        // An empty rates string has no textual default to inject, so it is read defensively
        return MG_ByType(sc.tryGet<string>("rates", ""), sc.get<double>("maintain_rate"),
                         sc.get<double>("default_rate"), sc.tryGet<string>("collateral_rates", ""),
                         sc.tryGet<double>("default_collateral_rate", 1.0));
    });
}

} /* namespace hku */
