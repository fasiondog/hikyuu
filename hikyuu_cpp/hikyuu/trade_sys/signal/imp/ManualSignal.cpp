/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualSignal.h"
#include "hikyuu/part/PartRegister.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ManualSignal)
#endif

namespace hku {

ManualSignal::ManualSignal() : SignalBase("SG_Manual") {
    m_keepSignals = true;
}

void ManualSignal::_calculate(const KData&) {}

SignalPtr HKU_API SG_Manual() {
    return make_shared<ManualSignal>();
}

void registerPart_SG_Manual() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.sg.manual"), void());
    PartMeta meta;
    meta.type = PartType::SG;
    meta.name = "SG_Manual";
    meta.full_name = "core.sg.manual";
    reg.registerPart(
      meta, [](const Parameter& sc, const PartParamMap& pp) -> Part { return SG_Manual(); });
}

}  // namespace hku