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
                              const TradeCostPtr& costfunc, const string& name) {
    return make_shared<TradeManager>(datetime, initcash, costfunc, name);
}

void registerPart_crtTM() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.tm.simple"), void());
    PartMeta meta;
    meta.type = PartType::TM;
    meta.name = "crtTM";
    meta.full_name = "core.tm.simple";
    meta.params.push_back(ParamMeta{.name = "date", .type = "Datetime"});
    meta.params.push_back(
      ParamMeta{.name = "init_cash", .type = "double", .default_value = "100000.0"});
    meta.params.push_back(
      ParamMeta{.name = "cost_func", .type = "TradeCost", .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "name", .type = "string", .default_value = "SYS"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return crtTM((sc.have("date") ? sc.get<Datetime>("date") : Datetime(199001010000LL)),
                     sc.get<double>("init_cash"), partOr<TradeCostPtr>(pp, "cost_func", TC_Zero()),
                     sc.get<string>("name"));
    });
}

}  // namespace hku
