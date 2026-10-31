/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualCondition.h"
#include "hikyuu/part/PartRegister.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ManualCondition)
#endif

namespace hku {

ManualCondition::ManualCondition() : ConditionBase("CN_Manual") {}

void ManualCondition::_calculate() {}

CNPtr HKU_API CN_Manual() {
    return make_shared<ManualCondition>();
}

void registerPart_CN_Manual() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.cn.manual"), void());
    PartMeta meta;
    meta.type = PartType::CN;
    meta.name = "CN_Manual";
    meta.full_name = "core.cn.manual";
    reg.registerPart(
      meta, [](const Parameter& sc, const PartParamMap& pp) -> Part { return CN_Manual(); });
}

}  // namespace hku