/*
 * PF_WithoutAF.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 *
 *  The PF factory implementation -- passes through to the MultiSystem preset configuration
 *  (mode C "Shared Account Compatibility", i.e. the legacy Portfolio without a fund allocation
 *  algorithm).
 */

#include "PF_WithoutAF.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

MultiSystemPtr HKU_API PF_WithoutAF(const TMPtr& tm, const SEPtr& se, int adjust_cycle,
                                    const string& adjust_mode, bool delay_to_trading_day,
                                    bool trade_on_close, bool sys_use_self_tm,
                                    bool sell_at_not_selected, bool keep_watch_force_sell) {
    auto sys = std::make_shared<MultiSystem>("PF_WithoutAF");
    // The preset: MultiSystem mode C "Shared Account Compatibility" (the legacy Portfolio: the
    // sub-systems trade on the shared real account and are sized by their own MM). The mode is
    // held by the aggregate system; the AF only provides the L1/L2/L3 no-ops here: L1 takes no
    // quota, L2 needs no conversion, L3 is off by default.
    sys->setAF(AF_EqualWeight());
    sys->setMode("C");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    sys->setTradeOnClose(trade_on_close);
    sys->setSellAtNotSelected(sell_at_not_selected);
    sys->setKeepWatchForceSell(keep_watch_force_sell);

    // Mode C always shares the real account of the parent, so the "own tm of the prototype
    // system" branch of the legacy engine is not exposed as a per-layer capability (C-O3=A)
    HKU_WARN_IF(sys_use_self_tm,
                "PF_WithoutAF: sys_use_self_tm is ignored (mode C always shares the real account "
                "of the parent)!");
    HKU_WARN_IF(!tm, "PF_WithoutAF: tm is null!");
    HKU_WARN_IF(!se, "PF_WithoutAF: se is null!");
    return sys;
}

void registerPart_PF_WithoutAF() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.pf.withoutaf"), void());
    PartMeta meta;
    meta.type = PartType::PF;
    meta.name = "PF_WithoutAF";
    meta.full_name = "core.pf.withoutaf";
    meta.params.push_back(ParamMeta{.name = "tm", .type = "TradeManager", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "se", .type = "Selector", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "adjust_cycle", .type = "int", .default_value = "1"});
    meta.params.push_back(
      ParamMeta{.name = "adjust_mode", .type = "string", .default_value = "query"});
    meta.params.push_back(
      ParamMeta{.name = "delay_to_trading_day", .type = "bool", .default_value = "true"});
    meta.params.push_back(
      ParamMeta{.name = "trade_on_close", .type = "bool", .default_value = "true"});
    meta.params.push_back(
      ParamMeta{.name = "sys_use_self_tm", .type = "bool", .default_value = "false"});
    meta.params.push_back(
      ParamMeta{.name = "sell_at_not_selected", .type = "bool", .default_value = "false"});
    meta.params.push_back(
      ParamMeta{.name = "keep_watch_force_sell", .type = "bool", .default_value = "false"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return PF_WithoutAF(
          partOr<TMPtr>(pp, "tm", TradeManagerPtr()), partOr<SEPtr>(pp, "se", SE_Fixed()),
          static_cast<int>(sc.get<int64_t>("adjust_cycle")), sc.get<string>("adjust_mode"),
          sc.get<bool>("delay_to_trading_day"), sc.get<bool>("trade_on_close"),
          sc.get<bool>("sys_use_self_tm"), sc.get<bool>("sell_at_not_selected"),
          sc.get<bool>("keep_watch_force_sell"));
    });
}

} /* namespace hku */
