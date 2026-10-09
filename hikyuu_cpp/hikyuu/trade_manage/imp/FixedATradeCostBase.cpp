/*
 * FixedATradeCostBase.cpp
 *
 *  Created on: 2026-10-9
 *      Author: fasiondog
 */

#include "FixedATradeCostBase.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/arithmetic.h"
#include "../../StockTypeInfo.h"

namespace hku {

FixedATradeCostBase::FixedATradeCostBase(const string& name) : TradeCostBase(name) {}

FixedATradeCostBase::~FixedATradeCostBase() {}

CostRecord FixedATradeCostBase::getBuyCost(const Datetime& datetime, const Stock& stock,
                                           price_t price, double num) const {
    CostRecord result;
    HKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    price_t value = price * num;
    HKU_IF_RETURN(value <= 0, result);

    int precision = stock.precision();
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }
    result.transferfee = _calcTransferFee(stock, value, num);
    result.total = result.commission + result.transferfee;
    return result;
}

CostRecord FixedATradeCostBase::getSellCost(const Datetime& datetime, const Stock& stock,
                                            price_t price, double num) const {
    CostRecord result;
    HKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    price_t value = price * num;
    HKU_IF_RETURN(value <= 0, result);

    int precision = stock.precision();
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    // Stamp duty applies to sells of A-shares, ChiNext, STAR and Beijing Stock Exchange stocks
    int type = stock.type();
    if (type == STOCKTYPE_A || type == STOCKTYPE_GEM || type == STOCKTYPE_START ||
        type == STOCKTYPE_A_BJ) {
        result.stamptax = roundEx(value * getParam<price_t>("stamptax"), precision);
    } else {
        result.stamptax = 0.0;
    }
    result.transferfee = _calcTransferFee(stock, value, num);
    result.others = 0.0;
    result.total = result.commission + result.stamptax + result.transferfee;
    return result;
}

void FixedATradeCostBase::_checkParam(const string& name) const {
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

} /* namespace hku */
