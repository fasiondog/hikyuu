/*
 * PF_Simple.cpp
 *
 *  Created on: 2018-1-13
 *      Author: fasiondog
 *
 *  The PF factory implementation -- passes through to the MultiSystem preset configuration
 *  (mode B "Fund Allocation").
 */

#include "PF_Simple.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

MultiSystemPtr HKU_API PF_Simple(const TMPtr& tm, const SEPtr& se, const AFPtr& af,
                                 int adjust_cycle, const string& adjust_mode,
                                 bool delay_to_trading_day, bool trade_on_close,
                                 bool sell_at_not_selected) {
    auto sys = std::make_shared<MultiSystem>("PF_Simple");
    // The preset: MultiSystem mode B "Fund Allocation" (the quota-first calibration on the
    // rebalancing day)
    sys->setAF(af ? af : AF_EqualWeight());
    sys->setMode("B");
    sys->setTM(tm);
    sys->setSE(se);
    sys->setAdjustCycle(adjust_cycle);
    sys->setAdjustMode(adjust_mode);
    sys->setDelayToTradingDay(delay_to_trading_day);
    // Aligned with master SimplePortfolio: execute at the close, force liquidating the unselected
    // (both are configurable now, the defaults keep the legacy behavior)
    sys->setTradeOnClose(trade_on_close);
    sys->setSellAtNotSelected(sell_at_not_selected);

    HKU_WARN_IF(!tm, "PF_Simple: tm is null!");
    HKU_WARN_IF(!se, "PF_Simple: se is null!");
    return sys;
}

void registerPart_PF_Simple() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.pf.simple"), void());
    PartMeta meta;
    meta.type = PartType::PF;
    meta.name = "PF_Simple";
    meta.full_name = "core.pf.simple";
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
      ParamMeta{.name = "sell_at_not_selected", .type = "bool", .default_value = "true"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return PF_Simple(partOr<TMPtr>(pp, "tm", TradeManagerPtr()),
                         partOr<SEPtr>(pp, "se", SE_Fixed()),
                         partOr<AFPtr>(pp, "af", AF_EqualWeight()),
                         static_cast<int>(sc.get<int64_t>("adjust_cycle")),
                         sc.get<string>("adjust_mode"), sc.get<bool>("delay_to_trading_day"),
                         sc.get<bool>("trade_on_close"), sc.get<bool>("sell_at_not_selected"));
    });
}

} /* namespace hku */
