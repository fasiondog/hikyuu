/*
 * AShareTradeCost.cpp
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#include "../../StockTypeInfo.h"
#include "FixedA2017TradeCost.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedA2017TradeCost)
#endif

namespace hku {

FixedA2017TradeCost::FixedA2017TradeCost() : TradeCostBase("TC_FixedA2017") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.00002);
}

FixedA2017TradeCost::~FixedA2017TradeCost() {}

CostRecord FixedA2017TradeCost::getBuyCost(const Datetime& datetime, const Stock& stock,
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

    result.transferfee = roundEx(value * getParam<price_t>("transferfee"), precision);
    result.total = result.commission + result.transferfee;

    return result;
}

CostRecord FixedA2017TradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
                                            price_t price, double num) const {
    CostRecord result;
    if (stock.isNull()) {
        HKU_WARN("Stock is NULL!");
        return result;
    }

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

TradeCostPtr FixedA2017TradeCost::_clone() {
    return TradeCostPtr(new FixedA2017TradeCost());
}

TradeCostPtr HKU_API TC_FixedA2017(price_t commission, price_t lowestCommission, price_t stamptax,
                                   price_t transferfee) {
    FixedA2017TradeCost* p = new FixedA2017TradeCost();
    p->setParam<price_t>("commission", commission);
    p->setParam<price_t>("lowest_commission", lowestCommission);
    p->setParam<price_t>("stamptax", stamptax);
    p->setParam<price_t>("transferfee", transferfee);
    return TradeCostPtr(p);
}

} /* namespace hku */
