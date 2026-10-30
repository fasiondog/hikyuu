/*
 * FixedValueSlippage.cpp
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */

#include <cmath>
#include "hikyuu/part/PartRegister.h"
#include <optional>
#include "FixedValueSlippage.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedValueSlippage)
#endif

namespace hku {

FixedValueSlippage::FixedValueSlippage() : SlippageBase("SP_FixedValue") {
    setParam<double>("value", 0.01);
    setParam<double>("short_value", 0.01);
}

FixedValueSlippage::~FixedValueSlippage() {}

void FixedValueSlippage::_checkParam(const string& name) const {
    if ("value" == name || "short_value" == name) {
        double value = getParam<double>(name);
        HKU_CHECK(value >= 0.0, "SP_FixedValue: param {} must be >= 0, got {}", name, value);
    }
}

price_t FixedValueSlippage::_getRealBuyPrice(const Datetime& datetime, price_t price) {
    return price + getParam<double>("value");
}

price_t FixedValueSlippage::_getRealSellPrice(const Datetime& datetime, price_t price) {
    return price - getParam<double>("value");
}

price_t FixedValueSlippage::_getRealBuyShortPrice(const Datetime& datetime, price_t price) {
    return price + getParam<double>("short_value");
}

price_t FixedValueSlippage::_getRealSellShortPrice(const Datetime& datetime, price_t price) {
    return price - getParam<double>("short_value");
}

void FixedValueSlippage::_calculate() {}

SlippagePtr HKU_API SP_FixedValue(double value, std::optional<double> short_value) {
    SlippagePtr ptr = make_shared<FixedValueSlippage>();
    ptr->setParam("value", value);
    ptr->setParam("short_value", short_value.value_or(value));
    return ptr;
}

void registerPart_SP_FixedValue() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.sp.fixedvalue"), void());
    PartMeta meta;
    meta.type = PartType::SP;
    meta.name = "SP_FixedValue";
    meta.full_name = "core.sp.fixedvalue";
    meta.params.push_back(ParamMeta{.name = "value", .type = "double", .default_value = "0.01"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return SP_FixedValue(sc.get<double>("value"), std::nullopt);
    });
}

} /* namespace hku */
