/*
 * PF_SignalAggregate.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-06
 *      Author: fasiondog
 *
 *  The PF factory implementation -- passes through to the MultiSystem preset configuration
 *  (mode A "Signal Aggregation").
 */

#include "PF_SignalAggregate.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

MultiSystemPtr HKU_API PF_SignalAggregate(const TMPtr& tm, const SEPtr& se, const AFPtr& af,
                                          int adjust_cycle, const string& adjust_mode,
                                          bool delay_to_trading_day, bool trade_on_close,
                                          bool sell_at_not_selected, price_t sub_init_cash) {
    auto sys = std::make_shared<MultiSystem>("PF_SignalAggregate");
    // The preset: MultiSystem mode A "Signal Aggregation" (the signal sources + the parent uniform
    // ordering). The shadow account of every sub-system is funded with sub_init_cash and runs
    // continuously (never reset), so a sub-system keeps submitting its current position intent.
    sys->setAF(af == nullptr ? AF_EqualWeight() : af);
    sys->setMode("A");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    sys->setTradeOnClose(trade_on_close);
    sys->setSellAtNotSelected(sell_at_not_selected);
    sys->setSubInitCash(sub_init_cash);

    HKU_WARN_IF(!tm, "PF_SignalAggregate: tm is null!");
    HKU_WARN_IF(!se, "PF_SignalAggregate: se is null!");
    return sys;
}

void registerPart_PF_SignalAggregate() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.pf.signalaggregate"), void());
    PartMeta meta;
    meta.type = PartType::PF;
    meta.name = "PF_SignalAggregate";
    meta.full_name = "core.pf.signalaggregate";
    meta.params.push_back(ParamMeta{.name = "tm", .type = "TradeManager", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "se", .type = "Selector", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "af", .type = "AllocateFunds", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "adjust_cycle", .type = "int", .default_value = "1"});
    meta.params.push_back(
      ParamMeta{.name = "adjust_mode", .type = "string", .default_value = "query"});
    meta.params.push_back(
      ParamMeta{.name = "delay_to_trading_day", .type = "bool", .default_value = "true"});
    meta.params.push_back(
      ParamMeta{.name = "trade_on_close", .type = "bool", .default_value = "true"});
    meta.params.push_back(
      ParamMeta{.name = "sell_at_not_selected", .type = "bool", .default_value = "false"});
    meta.params.push_back(
      ParamMeta{.name = "sub_init_cash", .type = "double", .default_value = "100000.0"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return PF_SignalAggregate(
          partOr<TMPtr>(pp, "tm", TradeManagerPtr()), partOr<SEPtr>(pp, "se", SE_Fixed()),
          partOr<AFPtr>(pp, "af", AF_EqualWeight()),
          static_cast<int>(sc.get<int64_t>("adjust_cycle")), sc.get<string>("adjust_mode"),
          sc.get<bool>("delay_to_trading_day"), sc.get<bool>("trade_on_close"),
          sc.get<bool>("sell_at_not_selected"), sc.get<double>("sub_init_cash"));
    });
}

} /* namespace hku */
