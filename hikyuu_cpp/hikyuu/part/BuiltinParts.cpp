/*
 * BuiltinParts.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "hikyuu/part/BuiltinParts.h"

#include "hikyuu/indicator/crt/MA.h"
#include "hikyuu/trade_manage/crt/TC_FixedETF.h"
#include "hikyuu/trade_manage/crt/crtTM.h"
#include "hikyuu/trade_sys/signal/crt/SG_Band.h"

namespace hku {

void registerBuiltinParts() {
    auto& reg = PartRegistry::instance();
    if (reg.has("core.ind.MA")) {
        return;
    }

    // ===== core.ind.MA =====
    PartMeta ma_meta;
    ma_meta.type = PartType::IND;
    ma_meta.name = "MA";
    ma_meta.full_name = "core.ind.MA";
    ma_meta.doc = "Moving average";
    ma_meta.result_num = 1;
    ma_meta.params.push_back(ParamMeta{.name = "n",
                                       .type = "int",
                                       .default_value = "22",
                                       .desc = "period window of the moving average",
                                       .required = false});

    reg.registerPart(ma_meta, [](const Parameter& sc, const PartParamMap&) -> Part {
        return MA(sc.get<int>("n"));
    });

    // ===== core.sg.SG_Band =====
    PartMeta band_meta;
    band_meta.type = PartType::SG;
    band_meta.name = "SG_Band";
    band_meta.full_name = "core.sg.SG_Band";
    band_meta.doc =
      "Band signal: buy when the indicator crosses above the upper band, "
      "sell when it crosses below the lower band";
    band_meta.params.push_back(ParamMeta{.name = "ind",
                                         .type = "Indicator",
                                         .desc = "indicator",
                                         .required = true,
                                         .is_part_param = true});
    band_meta.params.push_back(
      ParamMeta{.name = "lower", .type = "double", .desc = "lower band", .required = true});
    band_meta.params.push_back(
      ParamMeta{.name = "upper", .type = "double", .desc = "upper band", .required = true});

    reg.registerPart(band_meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return SG_Band(std::get<Indicator>(pp.at("ind").at(0)), sc.get<price_t>("lower"),
                       sc.get<price_t>("upper"));
    });

    // ===== core.tm.crtTM =====
    PartMeta tm_meta;
    tm_meta.type = PartType::TM;
    tm_meta.name = "crtTM";
    tm_meta.full_name = "core.tm.crtTM";
    tm_meta.doc = "Create the trade manager of the account";
    tm_meta.params.push_back(ParamMeta{
      .name = "datetime", .type = "Datetime", .desc = "account creation date", .required = false});
    tm_meta.params.push_back(ParamMeta{.name = "initcash",
                                       .type = "double",
                                       .default_value = "100000.0",
                                       .desc = "initial cash",
                                       .required = false});
    tm_meta.params.push_back(ParamMeta{.name = "name",
                                       .type = "string",
                                       .default_value = "SYS",
                                       .desc = "account name",
                                       .required = false});
    tm_meta.params.push_back(ParamMeta{.name = "costfunc",
                                       .type = "TradeCost",
                                       .desc = "trade cost algorithm, TC_Zero by default",
                                       .required = false,
                                       .is_part_param = true});

    reg.registerPart(tm_meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        TradeCostPtr cost = TC_Zero();
        auto cost_iter = pp.find("costfunc");
        if (cost_iter != pp.end() && !cost_iter->second.empty()) {
            cost = std::get<TradeCostPtr>(cost_iter->second.at(0));
        }
        return crtTM(sc.tryGet<Datetime>("datetime", Datetime(199001010000LL)),
                     sc.tryGet<price_t>("initcash", 100000.0), cost,
                     sc.tryGet<string>("name", "SYS"));
    });

    // ===== core.tc.TC_FixedETF =====
    PartMeta tc_meta;
    tc_meta.type = PartType::TC;
    tc_meta.name = "TC_FixedETF";
    tc_meta.full_name = "core.tc.TC_FixedETF";
    tc_meta.doc = "Fixed commission trade cost algorithm for ETF";
    tc_meta.params.push_back(ParamMeta{.name = "commission",
                                       .type = "double",
                                       .default_value = "0.0001",
                                       .desc = "commission ratio",
                                       .required = false});
    tc_meta.params.push_back(ParamMeta{.name = "lowestCommission",
                                       .type = "double",
                                       .default_value = "5.0",
                                       .desc = "lowest commission",
                                       .required = false});

    reg.registerPart(tc_meta, [](const Parameter& sc, const PartParamMap&) -> Part {
        return TC_FixedETF(sc.get<price_t>("commission"), sc.get<price_t>("lowestCommission"));
    });
}

}  // namespace hku
