/*
 * NoMargin.cpp
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#include "NoMargin.h"
#include "hikyuu/part/PartRegister.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NoMargin)
#endif

namespace hku {

NoMargin::NoMargin() : MarginBase("MG_None") {}

NoMargin::~NoMargin() {}

bool NoMargin::isMarginTrading(const Stock& stock) const {
    return false;
}

price_t NoMargin::getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                double num, MarginSide side) const {
    return nominal(stock, price, num);
}

MarginPtr NoMargin::_clone() {
    return make_shared<NoMargin>();
}

MarginPtr HKU_API MG_None() {
    return make_shared<NoMargin>();
}

void registerPart_MG_None() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.mg.none"), void());
    PartMeta meta;
    meta.type = PartType::MG;
    meta.name = "MG_None";
    meta.full_name = "core.mg.none";
    reg.registerPart(meta,
                     [](const Parameter& sc, const PartParamMap& pp) -> Part { return MG_None(); });
}

} /* namespace hku */
