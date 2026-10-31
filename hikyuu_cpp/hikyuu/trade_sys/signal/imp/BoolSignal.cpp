/*
 * BoolSignal.cpp
 *
 *  Created on: 2017-7-2
 *      Author: fasiondog
 */

#include <cmath>
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "BoolSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::BoolSignal)
#endif

namespace hku {

BoolSignal::BoolSignal() : SignalBase("SG_Bool") {}

BoolSignal::BoolSignal(const Indicator& buy, const Indicator& sell)
: SignalBase("SG_Bool"), m_bool_buy(buy), m_bool_sell(sell) {}

BoolSignal::~BoolSignal() {}

SignalPtr BoolSignal::_clone() {
    auto p = make_shared<BoolSignal>();
    p->m_bool_buy = m_bool_buy.clone();
    p->m_bool_sell = m_bool_sell.clone();
    return p;
}

void BoolSignal::_calculate(const KData& kdata) {
    // Align only on a size mismatch: ALIGN always rebuilds the buffer
    Indicator buy = m_bool_buy(kdata);
    if (buy.size() != kdata.size()) {
        buy = ALIGN(buy, kdata);
    }
    Indicator sell = m_bool_sell(kdata);
    if (sell.size() != kdata.size()) {
        sell = ALIGN(sell, kdata);
    }
    HKU_WARN_IF_RETURN(buy.size() != sell.size() || buy.size() != kdata.size(), void(),
                       "Failed to align! buy.size()({}), sell.size()({}), kdata.size()({})! "
                       "The signal calculation will be ignored.",
                       buy.size(), sell.size(), kdata.size());

    size_t discard = buy.discard() > sell.discard() ? buy.discard() : sell.discard();
    size_t total = buy.size();
    auto const* buydata = buy.data();
    auto const* selldata = sell.data();
    auto const* ks = kdata.data();
    for (size_t i = discard; i < total; ++i) {
        // Same-bar opposite conflict: sell wins (risk-first contract)
        if (std::isfinite(selldata[i]) && selldata[i] > 0.0) {
            _addSellSignal(ks[i].datetime);
        } else if (std::isfinite(buydata[i]) && buydata[i] > 0.0) {
            _addBuySignal(ks[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Bool(const Indicator& buy, const Indicator& sell, bool alternate) {
    auto p = make_shared<BoolSignal>(buy, sell);
    p->setParam<bool>("alternate", alternate);
    return p;
}

void registerPart_SG_Bool() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.sg.bool"), void());
    PartMeta meta;
    meta.type = PartType::SG;
    meta.name = "SG_Bool";
    meta.full_name = "core.sg.bool";
    meta.params.push_back(
      ParamMeta{.name = "buy", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(
      ParamMeta{.name = "sell", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "alternate", .type = "bool", .default_value = "true"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return SG_Bool(std::get<Indicator>(pp.at("buy").at(0)),
                       std::get<Indicator>(pp.at("sell").at(0)), sc.get<bool>("alternate"));
    });
}

} /* namespace hku */
