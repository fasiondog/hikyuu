/*
 * FixedMargin.cpp
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#include <cmath>
#include <algorithm>
#include "FixedMargin.h"
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/utilities/Log.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedMargin)
#endif

namespace hku {

namespace {
/** Margin ratio range: (0, 1] */
void checkRate(const string& name, double rate) {
    HKU_CHECK(rate > 0.0 && rate <= 1.0, "MG_Fixed: {}({}) must be in (0, 1]!", name, rate);
}
}  // namespace

FixedMargin::FixedMargin() : MarginBase("MG_Fixed") {
    setParam<double>("rate", 0.6);
    setParam<double>("rate_short", 0.0);
    setParam<double>("maintain_rate", 0.0);
    setParam<double>("margin_per_lot", 0.0);
}

FixedMargin::~FixedMargin() {}

void FixedMargin::_checkParam(const string&) const {
    // All the parameters are re-validated on any change, so the check does not depend on the
    // order in which the named params are set
    double rate = tryGetParam<double>("rate", 0.6);
    double rate_short = tryGetParam<double>("rate_short", 0.0);
    double maintain_rate = tryGetParam<double>("maintain_rate", 0.0);
    double margin_per_lot = tryGetParam<double>("margin_per_lot", 0.0);

    checkRate("rate", rate);
    if (rate_short > 0.0)
        checkRate("rate_short", rate_short);
    if (maintain_rate > 0.0) {
        checkRate("maintain_rate", maintain_rate);
        // The maintenance margin must stay below the opening margin of every side, otherwise the
        // account would be called the moment a position is opened
        double effective_short = rate_short > 0.0 ? rate_short : rate;
        double min_open_rate = std::min(rate, effective_short);
        HKU_CHECK(maintain_rate <= min_open_rate,
                  "MG_Fixed: maintain_rate({}) must be <= the opening rate({})!", maintain_rate,
                  min_open_rate);
    }
    HKU_CHECK(margin_per_lot >= 0.0 && std::isfinite(margin_per_lot),
              "MG_Fixed: margin_per_lot({}) must be a finite value >= 0!", margin_per_lot);
}

double FixedMargin::rateOf(MarginSide side) const {
    double rate = getParam<double>("rate");
    if (side == MarginSide::SHORT) {
        double rate_short = getParam<double>("rate_short");
        if (rate_short > 0.0)
            return rate_short;
    }
    return rate;
}

price_t FixedMargin::getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num, MarginSide side) const {
    double per_lot = getParam<double>("margin_per_lot");
    if (per_lot > 0.0) {
        if (!std::isfinite(price) || !std::isfinite(num) || price <= 0.0 || num <= 0.0)
            return 0.0;
        double atom = stock.minTradeNumber();
        if (!std::isfinite(atom) || atom <= 0.0)
            atom = 1.0;
        return per_lot * std::ceil(num / atom);
    }
    return rateOf(side) * nominal(stock, price, num);
}

double FixedMargin::getMarginRate(const Datetime& datetime, const Stock& stock,
                                  MarginSide side) const {
    // The per-lot mode has no meaningful ratio view
    if (getParam<double>("margin_per_lot") > 0.0)
        return 1.0;
    return rateOf(side);
}

price_t FixedMargin::getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                       double num, MarginSide side) const {
    price_t open = getOpenMargin(datetime, stock, price, num, side);
    double maintain_rate = getParam<double>("maintain_rate");
    if (maintain_rate <= 0.0)
        return open;
    return maintain_rate / getMarginRate(datetime, stock, side) * open;
}

MarginPtr FixedMargin::_clone() {
    return make_shared<FixedMargin>();
}

MarginPtr HKU_API MG_Fixed(double rate, double rate_short, double maintain_rate,
                           double margin_per_lot) {
    auto margin = make_shared<FixedMargin>();
    margin->setParam<double>("rate", rate);
    margin->setParam<double>("rate_short", rate_short);
    margin->setParam<double>("maintain_rate", maintain_rate);
    margin->setParam<double>("margin_per_lot", margin_per_lot);
    return margin;
}

void registerPart_MG_Fixed() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.mg.fixed"), void());
    PartMeta meta;
    meta.type = PartType::MG;
    meta.name = "MG_Fixed";
    meta.full_name = "core.mg.fixed";
    meta.params.push_back(ParamMeta{.name = "rate", .type = "double", .default_value = "0.6"});
    meta.params.push_back(
      ParamMeta{.name = "rate_short", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "maintain_rate", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "margin_per_lot", .type = "double", .default_value = "0.0"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return MG_Fixed(sc.get<double>("rate"), sc.get<double>("rate_short"),
                        sc.get<double>("maintain_rate"), sc.get<double>("margin_per_lot"));
    });
}

} /* namespace hku */
