/*
 * FixedETFTradeCost.cpp
 */

#include "FixedETFTradeCost.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedETFTradeCost)
#endif

namespace hku {

FixedETFTradeCost::FixedETFTradeCost() : TradeCostBase("TC_FixedETF") {
    setParam<price_t>("commission", 0.0001);
    setParam<price_t>("lowest_commission", 5.0);
}

FixedETFTradeCost::~FixedETFTradeCost() {}

void FixedETFTradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    }
}

CostRecord FixedETFTradeCost::getBuyCost(const Datetime& datetime, const Stock& stock,
                                         price_t price, double num) const {
    CostRecord result;
    HKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    int precision = stock.precision();
    price_t value = price * num;
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }
    result.transferfee = 0.0;
    result.total = result.commission;
    return result;
}

CostRecord FixedETFTradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const {
    CostRecord result;
    HKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    int precision = stock.precision();
    price_t value = price * num;
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }
    result.stamptax = 0.0;
    result.transferfee = 0.0;
    result.others = 0.0;
    result.total = result.commission;
    return result;
}

TradeCostPtr FixedETFTradeCost::_clone() {
    return make_shared<FixedETFTradeCost>();
}

TradeCostPtr HKU_API TC_FixedETF(price_t commission, price_t lowestCommission) {
    TradeCostPtr p = make_shared<FixedETFTradeCost>();
    p->setParam<price_t>("commission", commission);
    p->setParam<price_t>("lowest_commission", lowestCommission);
    return p;
}

} /* namespace hku */