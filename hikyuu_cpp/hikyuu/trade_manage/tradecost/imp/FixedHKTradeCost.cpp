/*
 * FixedHKTradeCost.cpp
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#include "FixedHKTradeCost.h"
#include <cmath>
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedHKTradeCost)
#endif

namespace hku {

FixedHKTradeCost::FixedHKTradeCost() : TradeCostBase("TC_HK") {
    setParam<price_t>("commission", 0.00025);
    setParam<price_t>("lowest_commission", 3.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("tradefee", 0.000085);
    setParam<price_t>("per_trade", 0.0);
    setParam<price_t>("settlefee", 0.0);
    setParam<price_t>("settlefee_min", 0.0);
    setParam<price_t>("settlefee_max", 0.0);
}

FixedHKTradeCost::FixedHKTradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                                   price_t tradefee, price_t perTrade, price_t settlefee,
                                   price_t settleFeeMin, price_t settleFeeMax)
: TradeCostBase("TC_HK") {
    setParam<price_t>("commission", commission);
    setParam<price_t>("lowest_commission", lowestCommission);
    setParam<price_t>("stamptax", stamptax);
    setParam<price_t>("tradefee", tradefee);
    setParam<price_t>("per_trade", perTrade);
    setParam<price_t>("settlefee", settlefee);
    setParam<price_t>("settlefee_min", settleFeeMin);
    setParam<price_t>("settlefee_max", settleFeeMax);
}

FixedHKTradeCost::~FixedHKTradeCost() {}

CostRecord FixedHKTradeCost::getCost(const Stock& stock, price_t price, double num) const {
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
    // The HKEX rounds the stamp duty up to a whole HK dollar per order; the 1e-9 tolerance keeps
    // the floating point noise (e.g. 10000 * 0.001 = 10.000000000000002) from adding a dollar
    price_t stamp = value * getParam<price_t>("stamptax");
    result.stamptax = stamp > 0.0 ? std::ceil(stamp - 1e-9) : 0.0;

    // The settlement fee is charged as a ratio and bounded per order; 0 means the bound is off
    price_t settle = value * getParam<price_t>("settlefee");
    if (settle > 0.0) {
        price_t settle_min = getParam<price_t>("settlefee_min");
        price_t settle_max = getParam<price_t>("settlefee_max");
        if (settle_min > 0.0 && settle < settle_min) {
            settle = settle_min;
        }
        if (settle_max > 0.0 && settle > settle_max) {
            settle = settle_max;
        }
    }
    result.transferfee = roundEx(value * getParam<price_t>("tradefee") + settle, precision);
    result.others = getParam<price_t>("per_trade");
    result.total = result.commission + result.stamptax + result.transferfee + result.others;
    return result;
}

CostRecord FixedHKTradeCost::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                        double num) const {
    return getCost(stock, price, num);
}

CostRecord FixedHKTradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
                                         price_t price, double num) const {
    return getCost(stock, price, num);
}

void FixedHKTradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    } else if ("stamptax" == name) {
        HKU_ASSERT(getParam<price_t>("stamptax") >= 0.0);
    } else if ("tradefee" == name) {
        HKU_ASSERT(getParam<price_t>("tradefee") >= 0.0);
    } else if ("per_trade" == name) {
        HKU_ASSERT(getParam<price_t>("per_trade") >= 0.0);
    } else if ("settlefee" == name) {
        HKU_ASSERT(getParam<price_t>("settlefee") >= 0.0);
    } else if ("settlefee_min" == name) {
        HKU_ASSERT(getParam<price_t>("settlefee_min") >= 0.0);
    } else if ("settlefee_max" == name) {
        HKU_ASSERT(getParam<price_t>("settlefee_max") >= 0.0);
    }
}

TradeCostPtr FixedHKTradeCost::_clone() {
    // clone() copies the parameters and the name over, only the defaults matter here
    return make_shared<FixedHKTradeCost>();
}

TradeCostPtr HKU_API TC_HK(price_t commission, price_t lowestCommission, price_t stamptax,
                           price_t tradefee, price_t perTrade, price_t settlefee,
                           price_t settleFeeMin, price_t settleFeeMax) {
    return make_shared<FixedHKTradeCost>(commission, lowestCommission, stamptax, tradefee, perTrade,
                                         settlefee, settleFeeMin, settleFeeMax);
}

void registerPart_TC_HK() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.tc.hk"), void());
    PartMeta meta;
    meta.type = PartType::TC;
    meta.name = "TC_HK";
    meta.full_name = "core.tc.hk";
    meta.params.push_back(
      ParamMeta{.name = "commission", .type = "double", .default_value = "0.00025"});
    meta.params.push_back(
      ParamMeta{.name = "lowest_commission", .type = "double", .default_value = "3.0"});
    meta.params.push_back(
      ParamMeta{.name = "stamptax", .type = "double", .default_value = "0.001"});
    meta.params.push_back(
      ParamMeta{.name = "tradefee", .type = "double", .default_value = "0.000085"});
    meta.params.push_back(ParamMeta{.name = "per_trade", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "settlefee", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "settlefee_min", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "settlefee_max", .type = "double", .default_value = "0.0"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        return TC_HK(sc.get<double>("commission"), sc.get<double>("lowest_commission"),
                     sc.get<double>("stamptax"), sc.get<double>("tradefee"),
                     sc.get<double>("per_trade"), sc.get<double>("settlefee"),
                     sc.get<double>("settlefee_min"), sc.get<double>("settlefee_max"));
    });
}

} /* namespace hku */
