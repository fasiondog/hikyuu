/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-13
 *      Author: fasiondog
 */

#include <cmath>
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "OneSideSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OneSideSignal)
#endif

namespace hku {

OneSideSignal::OneSideSignal() : SignalBase("SG_OneSide") {
    setParam<bool>("alternate", false);
    setParam<bool>("is_buy", true);  // A buy signal, otherwise a sell signal is added
}

OneSideSignal::OneSideSignal(const Indicator& ind, bool is_buy)
: SignalBase("SG_OneSide"), m_ind(ind.clone()) {
    setParam<bool>("alternate", false);
    setParam<bool>("is_buy", is_buy);
}

OneSideSignal::~OneSideSignal() {}

void OneSideSignal::_checkParam(const string& name) const {
    if (name == "alternate") {
        HKU_CHECK(!getParam<bool>(name), "alternate only be false!");
    }
}

SignalPtr OneSideSignal::_clone() {
    auto p = make_shared<OneSideSignal>();
    p->m_ind = m_ind.clone();
    return p;
}

void OneSideSignal::_calculate(const KData& kdata) {
    Indicator ind = m_ind(kdata);
    if (ind.size() != kdata.size()) {
        ind = ALIGN(ind, kdata);
        HKU_WARN_IF_RETURN(ind.size() != kdata.size(), void(),
                           "Failed to align ind.size()({}) to kdata.size()({})! The signal "
                           "calculation will be ignored.",
                           ind.size(), kdata.size());
    }

    bool is_buy = getParam<bool>("is_buy");
    const auto* src = ind.data();
    auto const* ks = kdata.data();
    size_t discard = ind.discard();
    size_t total = ind.size();

    for (size_t i = discard; i < total; ++i) {
        if (std::isfinite(src[i]) && src[i] > 0.0) {
            if (is_buy) {
                _addBuySignal(ks[i].datetime);
            } else {
                _addSellSignal(ks[i].datetime);
            }
        }
    }
}

SignalPtr HKU_API SG_OneSide(const Indicator& ind, bool is_buy) {
    return make_shared<OneSideSignal>(ind, is_buy);
}

void registerPart_SG_OneSide() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.sg.oneside"), void());
    PartMeta meta;
    meta.type = PartType::SG;
    meta.name = "SG_OneSide";
    meta.full_name = "core.sg.oneside";
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "is_buy", .type = "bool", .required = true});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return SG_OneSide(std::get<Indicator>(pp.at("ind").at(0)), sc.get<bool>("is_buy"));
    });
}

} /* namespace hku */
