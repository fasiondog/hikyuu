/*
 * FixedA2015TradeCost.cpp
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */

#include "FixedA2015TradeCost.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedA2015TradeCost)
#endif

namespace hku {

FixedA2015TradeCost::FixedA2015TradeCost() : FixedATradeCostBase("TC_FixedA2015") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.00002);
}

FixedA2015TradeCost::~FixedA2015TradeCost() {}

price_t FixedA2015TradeCost::_calcTransferFee(const Stock& stock, price_t value, double) const {
    // Before 2017 only the Shanghai market charges the transfer fee (by turnover value)
    if (stock.market() != "SH") {
        return 0.0;
    }
    return roundEx(value * getParam<price_t>("transferfee"), stock.precision());
}

TradeCostPtr FixedA2015TradeCost::_clone() {
    return make_shared<FixedA2015TradeCost>();
}

TradeCostPtr HKU_API TC_FixedA2015(price_t commission, price_t lowestCommission, price_t stamptax,
                                   price_t transferfee) {
    TradeCostPtr p = make_shared<FixedA2015TradeCost>();
    p->setParam<price_t>("commission", commission);
    p->setParam<price_t>("lowest_commission", lowestCommission);
    p->setParam<price_t>("stamptax", stamptax);
    p->setParam<price_t>("transferfee", transferfee);
    return p;
}

} /* namespace hku */
