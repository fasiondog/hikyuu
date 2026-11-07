/*
 * FixedTradeCost.cpp
 *
 *  Created on: 2026-11-07
 *      Author: fasiondog
 */

#include "FixedTradeCost.h"
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedTradeCost)
#endif

namespace hku {

FixedTradeCost::FixedTradeCost() : TradeCostBase("TC_Fixed") {
    setParam<price_t>("commission", 0.0);
    setParam<price_t>("lowest_commission", 0.0);
    setParam<price_t>("stamptax", 0.0);
    setParam<price_t>("per_lot", 0.0);
    setParam<price_t>("others", 0.0);
}

FixedTradeCost::FixedTradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                               price_t perLot, price_t others)
: TradeCostBase("TC_Fixed") {
    setParam<price_t>("commission", commission);
    setParam<price_t>("lowest_commission", lowestCommission);
    setParam<price_t>("stamptax", stamptax);
    setParam<price_t>("per_lot", perLot);
    setParam<price_t>("others", others);
}

FixedTradeCost::~FixedTradeCost() {}

CostRecord FixedTradeCost::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num) const {
    CostRecord result;
    HKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    price_t value = price * num * stock.unit();
    HKU_IF_RETURN(value <= 0.0, result);

    int precision = stock.precision();
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowest_commission = getParam<price_t>("lowest_commission");
    if (result.commission < lowest_commission) {
        result.commission = lowest_commission;
    }
    result.others = getParam<price_t>("per_lot") * num + getParam<price_t>("others");
    result.total = result.commission + result.others;
    return result;
}

CostRecord FixedTradeCost::getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                       double num) const {
    CostRecord result;
    HKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    price_t value = price * num * stock.unit();
    HKU_IF_RETURN(value <= 0.0, result);

    int precision = stock.precision();
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowest_commission = getParam<price_t>("lowest_commission");
    if (result.commission < lowest_commission) {
        result.commission = lowest_commission;
    }
    result.stamptax = roundEx(value * getParam<price_t>("stamptax"), precision);
    result.others = getParam<price_t>("per_lot") * num + getParam<price_t>("others");
    result.total = result.commission + result.stamptax + result.others;
    return result;
}

void FixedTradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    } else if ("stamptax" == name) {
        HKU_ASSERT(getParam<price_t>("stamptax") >= 0.0);
    } else if ("per_lot" == name) {
        HKU_ASSERT(getParam<price_t>("per_lot") >= 0.0);
    } else if ("others" == name) {
        HKU_ASSERT(getParam<price_t>("others") >= 0.0);
    }
}

TradeCostPtr FixedTradeCost::_clone() {
    // clone() copies the parameters and the name over, only the defaults matter here
    return make_shared<FixedTradeCost>();
}

TradeCostPtr HKU_API TC_Fixed(price_t commission, price_t lowestCommission, price_t stamptax,
                              price_t perLot, price_t others) {
    return make_shared<FixedTradeCost>(commission, lowestCommission, stamptax, perLot, others);
}

void registerPart_TC_Fixed() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.tc.fixed"), void());
    PartMeta meta;
    meta.type = PartType::TC;
    meta.name = "TC_Fixed";
    meta.full_name = "core.tc.fixed";
    meta.params.push_back(
      ParamMeta{.name = "commission", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "lowest_commission", .type = "double", .default_value = "0.0"});
    meta.params.push_back(ParamMeta{.name = "stamptax", .type = "double", .default_value = "0.0"});
    meta.params.push_back(ParamMeta{.name = "per_lot", .type = "double", .default_value = "0.0"});
    meta.params.push_back(ParamMeta{.name = "others", .type = "double", .default_value = "0.0"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return TC_Fixed(sc.get<double>("commission"), sc.get<double>("lowest_commission"),
                        sc.get<double>("stamptax"), sc.get<double>("per_lot"),
                        sc.get<double>("others"));
    });
}

} /* namespace hku */
