/*
 * FixedATradeCost.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "FixedATradeCost.h"
#include "../../Log.h"

namespace hku {

FixedATradeCost::FixedATradeCost(): TradeCostBase("FixedATradeCost") {
    addParam<price_t>("commission", 0.0018);
    addParam<price_t>("lowest_commission", 5.0);
    addParam<price_t>("stamptax", 0.001);
    addParam<price_t>("transferfee", 0.001);
    addParam<price_t>("lowest_transferfee", 1.0);
}


FixedATradeCost::FixedATradeCost(
        price_t commission,
        price_t lowestCommission,
        price_t stamptax,
        price_t transferfee,
        price_t lowestTransferfee)
: TradeCostBase("FixedATradeCost") {
    addParam<price_t>("commission", commission);
    addParam<price_t>("lowest_commission", lowestCommission);
    addParam<price_t>("stamptax", stamptax);
    addParam<price_t>("transferfee", transferfee);
    addParam<price_t>("lowest_transferfee", lowestTransferfee);
}


FixedATradeCost::~FixedATradeCost() {

}


CostRecord FixedATradeCost::getBuyCost(const Datetime& datetime,
        const Stock& stock, price_t price, size_t num) const {
    CostRecord result;
    if (stock.isNull()) {
        HKU_WARN("Stock is Null! [FixedATradeCost::getBuyCost]");
        return result;
    }

    int precision = stock.precision();
    result.commission = roundEx(price * num * getParam<price_t>("commission"),
                                precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    //上证买入时，有过户费
    if (stock.market() == "SH") {
        result.transferfee = num > 1000
                ? roundEx(getParam<price_t>("transferfee") * num, precision)
                : getParam<price_t>("lowest_transferfee");
    }

    result.total = result.commission + result.transferfee;

    return result;
}


CostRecord FixedATradeCost::getSellCost(const Datetime& datetime,
        const Stock& stock, price_t price, size_t num) const {
    CostRecord result;
    if( stock.isNull() ){
        HKU_WARN("Stock is NULL! [FixedATradeCost::getBuyCost]");
        return result;
    }

    int precision = stock.precision();
    result.commission = roundEx(price * num * getParam<price_t>("commission"),
                                precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if( result.commission < lowestCommission ){
        result.commission = lowestCommission;
    }
    result.stamptax = roundEx(price * num * getParam<price_t>("stamptax"),
                              precision);
    result.transferfee = 0.0;
    if(stock.market() == "SH"){
        result.transferfee = num > 1000
                ? roundEx(getParam<price_t>("transferfee") * num, precision)
                : getParam<price_t>("lowest_transferfee");
    }
    result.others = 0.0;
    result.total = result.commission + result.stamptax + result.transferfee;
    return result;
}


TradeCostPtr FixedATradeCost::_clone() {
    return TradeCostPtr(new FixedATradeCost());
}

} /* namespace hku */
