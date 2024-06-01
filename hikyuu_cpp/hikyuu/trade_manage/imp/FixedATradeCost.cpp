/*
 * FixedATradeCost.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "FixedATradeCost.h"
#include "../../StockTypeInfo.h"
#include "../../Log.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedATradeCost)
#endif

namespace hku {

FixedATradeCost::FixedATradeCost() : TradeCostBase("TC_FixedA") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.001);
    setParam<price_t>("lowest_transferfee", 1.0);
}

FixedATradeCost::FixedATradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                                 price_t transferfee, price_t lowestTransferfee)
: TradeCostBase("FixedATradeCost") {
    setParam<price_t>("commission", commission);
    setParam<price_t>("lowest_commission", lowestCommission);
    setParam<price_t>("stamptax", stamptax);
    setParam<price_t>("transferfee", transferfee);
    setParam<price_t>("lowest_transferfee", lowestTransferfee);
}

FixedATradeCost::~FixedATradeCost() {}

void FixedATradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    } else if ("stamptax" == name) {
        HKU_ASSERT(getParam<price_t>("stamptax") >= 0.0);
    } else if ("transferfee" == name) {
        HKU_ASSERT(getParam<price_t>("transferfee") >= 0.0);
    } else if ("lowest_transferfee" == name) {
        HKU_ASSERT(getParam<price_t>("lowest_transferfee") >= 0.0);
    }
}

CostRecord FixedATradeCost::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                       double num) const {
    CostRecord result;
    HKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    int precision = stock.precision();
    result.commission = roundEx(price * num * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    // 上证买入时，有过户费
    if (stock.market() == "SH") {
        result.transferfee = num > 1000 ? roundEx(getParam<price_t>("transferfee") * num, precision)
                                        : getParam<price_t>("lowest_transferfee");
    }

    result.total = result.commission + result.transferfee;

    return result;
}

CostRecord FixedATradeCost::getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                        double num) const {
    CostRecord result;
    if (stock.isNull()) {
        HKU_WARN("Stock is NULL!");
        return result;
    }

    int precision = stock.precision();
    result.commission = roundEx(price * num * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    // A股和创业板有印花税，其他无
    if (stock.type() == STOCKTYPE_A || stock.type() == STOCKTYPE_GEM) {
        result.stamptax = roundEx(price * num * getParam<price_t>("stamptax"), precision);
    } else {
        result.stamptax = 0.0;
    }
    result.transferfee = 0.0;
    if (stock.market() == "SH") {
        result.transferfee = num > 1000 ? roundEx(getParam<price_t>("transferfee") * num, precision)
                                        : getParam<price_t>("lowest_transferfee");
    }
    result.others = 0.0;
    result.total = result.commission + result.stamptax + result.transferfee;
    return result;
}

TradeCostPtr FixedATradeCost::_clone() {
    return make_shared<FixedATradeCost>();
}

TradeCostPtr HKU_API TC_FixedA(price_t commission, price_t lowestCommission, price_t stamptax,
                               price_t transferfee, price_t lowestTransferfee) {
    return make_shared<FixedATradeCost>(commission, lowestCommission, stamptax, transferfee,
                                        lowestTransferfee);
}

} /* namespace hku */
