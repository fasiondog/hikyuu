/*
 * SYS_Simple.cpp
 *
 *  Created on: 2013-5-7
 *      Author: fasiondog
 */

#include "../crt/SYS_Simple.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

SystemPtr HKU_API SYS_Simple(const TradeManagerPtr& tm, const MoneyManagerPtr& mm,
                             const EnvironmentPtr& ev, const ConditionPtr& cn, const SignalPtr& sg,
                             const StoplossPtr& st, const StoplossPtr& tp, const ProfitGoalPtr& pg,
                             const SlippagePtr& sp) {
    return make_shared<System>(tm, mm, ev, cn, sg, st, tp, pg, sp, "SYS_Simple");
}

void registerPart_SYS_Simple() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.sys.simple"), void());
    PartMeta meta;
    meta.type = PartType::SYS;
    meta.name = "SYS_Simple";
    meta.full_name = "core.sys.simple";
    meta.params.push_back(ParamMeta{.name = "tm", .type = "TradeManager", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "mm", .type = "MoneyManager", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "ev", .type = "Environment", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "cn", .type = "Condition", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "sg", .type = "Signal", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "sl", .type = "Stoploss", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "tp", .type = "Stoploss", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "pg", .type = "ProfitGoal", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "sp", .type = "Slippage", .is_part_param = true});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return SYS_Simple(partOr<TradeManagerPtr>(pp, "tm", TradeManagerPtr()),
                          partOr<MoneyManagerPtr>(pp, "mm", MoneyManagerPtr()),
                          partOr<EnvironmentPtr>(pp, "ev", EnvironmentPtr()),
                          partOr<ConditionPtr>(pp, "cn", ConditionPtr()),
                          partOr<SignalPtr>(pp, "sg", SignalPtr()),
                          partOr<StoplossPtr>(pp, "sl", StoplossPtr()),
                          partOr<StoplossPtr>(pp, "tp", StoplossPtr()),
                          partOr<ProfitGoalPtr>(pp, "pg", ProfitGoalPtr()),
                          partOr<SlippagePtr>(pp, "sp", SlippagePtr()));
    });
}

} /* namespace hku */
