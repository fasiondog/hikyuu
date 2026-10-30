/*
 * FixedATradeCost.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "FixedATradeCost.h"
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedATradeCost)
#endif

namespace hku {

FixedATradeCost::FixedATradeCost() : FixedATradeCostBase("TC_FixedA") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.0005);
    setParam<price_t>("transferfee", 0.00001);
}

FixedATradeCost::FixedATradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                                 price_t transferfee)
: FixedATradeCostBase("FixedATradeCost") {
    setParam<price_t>("commission", commission);
    setParam<price_t>("lowest_commission", lowestCommission);
    setParam<price_t>("stamptax", stamptax);
    setParam<price_t>("transferfee", transferfee);
}

FixedATradeCost::~FixedATradeCost() {}

price_t FixedATradeCost::_calcTransferFee(const Stock& stock, price_t value, double) const {
    // Transfer fee by turnover value, charged on all markets in both directions
    return roundEx(value * getParam<price_t>("transferfee"), stock.precision());
}

TradeCostPtr FixedATradeCost::_clone() {
    return make_shared<FixedATradeCost>();
}

TradeCostPtr HKU_API TC_FixedA(price_t commission, price_t lowestCommission, price_t stamptax,
                               price_t transferfee) {
    return make_shared<FixedATradeCost>(commission, lowestCommission, stamptax, transferfee);
}

void registerPart_TC_FixedA() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.tc.fixeda"), void());
    PartMeta meta;
    meta.type = PartType::TC;
    meta.name = "TC_FixedA";
    meta.full_name = "core.tc.fixeda";
    meta.params.push_back(
      ParamMeta{.name = "commission", .type = "double", .default_value = "0.0018"});
    meta.params.push_back(
      ParamMeta{.name = "lowestCommission", .type = "double", .default_value = "5.0"});
    meta.params.push_back(
      ParamMeta{.name = "stamptax", .type = "double", .default_value = "0.0005"});
    meta.params.push_back(
      ParamMeta{.name = "transferfee", .type = "double", .default_value = "0.00001"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return TC_FixedA(sc.get<double>("commission"), sc.get<double>("lowestCommission"),
                         sc.get<double>("stamptax"), sc.get<double>("transferfee"));
    });
}

} /* namespace hku */
