/*
 * Saftloss_ST.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include <cmath>
#include "hikyuu/part/PartRegister.h"
#include "ST_Saftyloss.h"
#include "../../../indicator/crt/SAFTYLOSS.h"
#include "../../../indicator/crt/KDATA.h"

namespace hku {

StoplossPtr HKU_API ST_Saftyloss(int n1, int n2, double p) {
    HKU_CHECK(std::isfinite(p) && p > 0.0, "p must be finite and greater than 0, but got {}", p);
    Indicator op = SAFTYLOSS(CLOSE(), n1, n2, p);
    auto result = make_shared<IndicatorStoploss>(op);
    result->name("Saftyloss_ST");
    return result;
}

void registerPart_ST_Saftyloss() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.st.saftyloss"), void());
    PartMeta meta;
    meta.type = PartType::ST;
    meta.name = "ST_Saftyloss";
    meta.full_name = "core.st.saftyloss";
    meta.params.push_back(ParamMeta{.name = "n1", .type = "int", .default_value = "10"});
    meta.params.push_back(ParamMeta{.name = "n2", .type = "int", .default_value = "3"});
    meta.params.push_back(ParamMeta{.name = "p", .type = "double", .default_value = "2.0"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return ST_Saftyloss(static_cast<int>(sc.get<int64_t>("n1")),
                            static_cast<int>(sc.get<int64_t>("n2")), sc.get<double>("p"));
    });
}

}  // namespace hku
