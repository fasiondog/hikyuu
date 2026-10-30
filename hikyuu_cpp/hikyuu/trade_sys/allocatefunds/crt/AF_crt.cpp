/*
 * AF_crt.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The AF factory implementation: assemble the concrete implementations of AllocateFundsBase (the
 * three parts L1/L2/L3).
 */

#include "AF_EqualWeight.h"
#include "hikyuu/part/PartRegister.h"
#include "AF_FixedWeight.h"
#include "AF_FixedWeightList.h"
#include "AF_FixedAmount.h"
#include "AF_MultiFactor.h"

#include "../imp/EqualWeightAllocateFunds.h"
#include "../imp/FixedWeightAllocateFunds.h"
#include "../imp/FixedWeightListAllocateFunds.h"
#include "../imp/FixedAmountAllocateFunds.h"
#include "../imp/MultiFactorAllocateFunds.h"

namespace hku {

AFPtr HKU_API AF_EqualWeight() {
    // The equal weight = the L1 default 1/N (equivalent to the normalized equal-proportion
    // semantics of master EqualWeightAllocateFunds)
    return std::make_shared<EqualWeightAllocateFunds>();
}

AFPtr HKU_API AF_FixedWeight(double weight) {
    auto p = std::make_shared<FixedWeightAllocateFunds>();
    p->setParam<double>("weight", weight);
    return p;
}

AFPtr HKU_API AF_FixedWeightList(const vector<double>& weights) {
    HKU_CHECK(!weights.empty(), "The weights list must not be empty!");
    auto p = std::make_shared<FixedWeightListAllocateFunds>();
    p->setParam<PriceList>("weights", PriceList(weights.begin(), weights.end()));
    return p;
}

AFPtr HKU_API AF_FixedAmount(double amount) {
    auto p = std::make_shared<FixedAmountAllocateFunds>();
    p->setParam<double>("fixed_amount", amount);
    return p;
}

AFPtr HKU_API AF_MultiFactor(bool auto_adjust_weight) {
    auto p = std::make_shared<MultiFactorAllocateFunds>();
    p->setParam<bool>("auto_adjust_weight", auto_adjust_weight);
    return p;
}

void registerPart_AF_FixedWeightList() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.af.fixedweightlist"), void());
    PartMeta meta;
    meta.type = PartType::AF;
    meta.name = "AF_FixedWeightList";
    meta.full_name = "core.af.fixedweightlist";
    meta.params.push_back(ParamMeta{.name = "weights", .type = "PriceList", .required = true});
    reg.registerPart(meta, [](const Parameter&, const PartParamMap&) -> Part {
        HKU_THROW_EXCEPTION(std::logic_error,
                            "AF_FixedWeightList takes a weight list, create it through the "
                            "programming interface");
        return AFPtr();
    });
}

void registerPart_AF_MultiFactor() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.af.multifactor"), void());
    PartMeta meta;
    meta.type = PartType::AF;
    meta.name = "AF_MultiFactor";
    meta.full_name = "core.af.multifactor";
    meta.params.push_back(
      ParamMeta{.name = "auto_adjust_weight", .type = "bool", .default_value = "true"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return AF_MultiFactor(sc.get<bool>("auto_adjust_weight"));
    });
}

void registerPart_AF_FixedWeight() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.af.fixedweight"), void());
    PartMeta meta;
    meta.type = PartType::AF;
    meta.name = "AF_FixedWeight";
    meta.full_name = "core.af.fixedweight";
    meta.params.push_back(ParamMeta{.name = "weight", .type = "double", .default_value = "0.1"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return AF_FixedWeight(sc.get<double>("weight"));
    });
}

void registerPart_AF_FixedAmount() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.af.fixedamount"), void());
    PartMeta meta;
    meta.type = PartType::AF;
    meta.name = "AF_FixedAmount";
    meta.full_name = "core.af.fixedamount";
    meta.params.push_back(
      ParamMeta{.name = "amount", .type = "double", .default_value = "20000.0"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return AF_FixedAmount(sc.get<double>("amount"));
    });
}

void registerPart_AF_EqualWeight() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.af.equalweight"), void());
    PartMeta meta;
    meta.type = PartType::AF;
    meta.name = "AF_EqualWeight";
    meta.full_name = "core.af.equalweight";
    reg.registerPart(
      meta, [](const Parameter& sc, const PartParamMap& pp) -> Part { return AF_EqualWeight(); });
}

}  // namespace hku
