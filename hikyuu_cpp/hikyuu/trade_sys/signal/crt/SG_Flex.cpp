/*
 * SG_Flex.cpp
 *
 *  Created on: 2016-4-11
 *      Author: fasiondog
 */

#include "../../../indicator/crt/EMA.h"
#include "hikyuu/part/PartRegister.h"
#include "SG_Cross.h"

namespace hku {

SignalPtr HKU_API SG_Flex(const Indicator& op, int slow_n) {
    SignalPtr sg = SG_Cross(op, EMA(slow_n)(op));
    sg->name("SG_Flex");
    return sg;
}

void registerPart_SG_Flex() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.sg.flex"), void());
    PartMeta meta;
    meta.type = PartType::SG;
    meta.name = "SG_Flex";
    meta.full_name = "core.sg.flex";
    meta.params.push_back(
      ParamMeta{.name = "op", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "slow_n", .type = "int", .required = true});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return SG_Flex(std::get<Indicator>(pp.at("op").at(0)),
                       static_cast<int>(sc.get<int64_t>("slow_n")));
    });
}

} /* namespace hku */
