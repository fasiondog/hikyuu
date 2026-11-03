/*
 * crtTM.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include "crtTM.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

TradeManagerPtr HKU_API crtTM(const Datetime& datetime, price_t initcash,
                              const TradeCostPtr& costfunc, const string& name,
                              const MarginPtr& margin) {
    auto tm = make_shared<TradeManager>(datetime, initcash, costfunc, name);
    // A null margin keeps the default model of the account itself
    if (margin) {
        tm->margin(margin);
    }
    return tm;
}

void registerPart_crtTM() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.tm.simple"), void());
    PartMeta meta;
    meta.type = PartType::TM;
    meta.name = "crtTM";
    meta.full_name = "core.tm.simple";
    meta.params.push_back(ParamMeta{.name = "datetime", .type = "Datetime"});
    meta.params.push_back(
      ParamMeta{.name = "initcash", .type = "double", .default_value = "100000.0"});
    meta.params.push_back(
      ParamMeta{.name = "costfunc", .type = "TradeCost", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "name", .type = "string", .default_value = "SYS"});
    meta.params.push_back(ParamMeta{.name = "margin", .type = "Margin", .is_part_param = true});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return crtTM(
          (sc.have("datetime") ? sc.get<Datetime>("datetime") : Datetime(199001010000LL)),
          sc.get<double>("initcash"), partOr<TradeCostPtr>(pp, "costfunc", TC_Zero()),
          sc.get<string>("name"), partOr<MarginPtr>(pp, "margin", MarginPtr()));
    });
}

}  // namespace hku
