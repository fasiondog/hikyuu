/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualEnvironment.h"
#include "hikyuu/part/PartRegister.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ManualEnvironment)
#endif

namespace hku {

ManualEnvironment::ManualEnvironment() : EnvironmentBase("EV_Manual") {}

void ManualEnvironment::_calculate() {}

EVPtr HKU_API EV_Manual() {
    return make_shared<ManualEnvironment>();
}

void registerPart_EV_Manual() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.ev.manual"), void());
    PartMeta meta;
    meta.type = PartType::EV;
    meta.name = "EV_Manual";
    meta.full_name = "core.ev.manual";
    reg.registerPart(
      meta, [](const Parameter& sc, const PartParamMap& pp) -> Part { return EV_Manual(); });
}

}  // namespace hku