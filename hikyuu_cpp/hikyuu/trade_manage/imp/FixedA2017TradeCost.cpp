/*
 * FixedA2017TradeCost.cpp
 *
 *  Created on: 2018-4-11
 *      Author: fasiondog
 */

#include "FixedA2017TradeCost.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedA2017TradeCost)
#endif

namespace hku {

FixedA2017TradeCost::FixedA2017TradeCost() : FixedATradeCostBase("TC_FixedA2017") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.00002);
}

FixedA2017TradeCost::~FixedA2017TradeCost() {}

price_t FixedA2017TradeCost::_calcTransferFee(const Stock& stock, price_t value, double) const {
    // From 2017 the Shenzhen market also charges the transfer fee, in both directions
    return roundEx(value * getParam<price_t>("transferfee"), stock.precision());
}

TradeCostPtr FixedA2017TradeCost::_clone() {
    return make_shared<FixedA2017TradeCost>();
}

TradeCostPtr HKU_API TC_FixedA2017(price_t commission, price_t lowestCommission, price_t stamptax,
                                   price_t transferfee) {
    TradeCostPtr p = make_shared<FixedA2017TradeCost>();
    p->setParam<price_t>("commission", commission);
    p->setParam<price_t>("lowest_commission", lowestCommission);
    p->setParam<price_t>("stamptax", stamptax);
    p->setParam<price_t>("transferfee", transferfee);
    return p;
}

} /* namespace hku */
