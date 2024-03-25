/*
 * AShareTradeCost.cpp
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#include "../../StockTypeInfo.h"
#include "FixedA2015TradeCost.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedA2015TradeCost)
#endif

namespace hku {

FixedA2015TradeCost::FixedA2015TradeCost() : TradeCostBase("TC_FixedA2015") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.00002);
}

FixedA2015TradeCost::~FixedA2015TradeCost() {}

void FixedA2015TradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    } else if ("stamptax" == name) {
        HKU_ASSERT(getParam<price_t>("stamptax") >= 0.0);
    } else if ("transferfee" == name) {
        HKU_ASSERT(getParam<price_t>("transferfee") >= 0.0);
    }
}

CostRecord FixedA2015TradeCost::getBuyCost(const Datetime& datetime, const Stock& stock,
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

    if (stock.market() == "SH") {
        result.transferfee = roundEx(value * getParam<price_t>("transferfee"), precision);
    }

    result.total = result.commission + result.transferfee;

    return result;
}

CostRecord FixedA2015TradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
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

    // A股和创业板有印花税，其他无
    if (stock.type() == STOCKTYPE_A || stock.type() == STOCKTYPE_GEM) {
        result.stamptax = roundEx(value * getParam<price_t>("stamptax"), precision);
    } else {
        result.stamptax = 0.0;
    }
    result.transferfee = 0.0;
    if (stock.market() == "SH") {
        result.transferfee = roundEx(value * getParam<price_t>("transferfee"), precision);
    }
    result.others = 0.0;
    result.total = result.commission + result.stamptax + result.transferfee;
    return result;
}

TradeCostPtr FixedA2015TradeCost::_clone() {
    return TradeCostPtr(new FixedA2015TradeCost());
}

TradeCostPtr HKU_API TC_FixedA2015(price_t commission, price_t lowestCommission, price_t stamptax,
                                   price_t transferfee) {
    FixedA2015TradeCost* p = new FixedA2015TradeCost();
    p->setParam<price_t>("commission", commission);
    p->setParam<price_t>("lowest_commission", lowestCommission);
    p->setParam<price_t>("stamptax", stamptax);
    p->setParam<price_t>("transferfee", transferfee);
    return TradeCostPtr(p);
}

} /* namespace hku */
