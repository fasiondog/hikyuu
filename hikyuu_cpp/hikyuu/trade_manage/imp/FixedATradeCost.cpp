/*
 * FixedATradeCost.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "FixedATradeCost.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedATradeCost)
#endif

namespace hku {

FixedATradeCost::FixedATradeCost() : FixedATradeCostBase("TC_FixedA") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.0005);
    setParam<price_t>("transferfee", 0.00001);
}

FixedATradeCost::FixedATradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                                 price_t transferfee)
: FixedATradeCostBase("FixedATradeCost") {
    setParam<price_t>("commission", commission);
    setParam<price_t>("lowest_commission", lowestCommission);
    setParam<price_t>("stamptax", stamptax);
    setParam<price_t>("transferfee", transferfee);
}

FixedATradeCost::~FixedATradeCost() {}

price_t FixedATradeCost::_calcTransferFee(const Stock& stock, price_t value, double) const {
    // Transfer fee by turnover value, charged on all markets in both directions
    return roundEx(value * getParam<price_t>("transferfee"), stock.precision());
}

TradeCostPtr FixedATradeCost::_clone() {
    return make_shared<FixedATradeCost>();
}

TradeCostPtr HKU_API TC_FixedA(price_t commission, price_t lowestCommission, price_t stamptax,
                               price_t transferfee) {
    return make_shared<FixedATradeCost>(commission, lowestCommission, stamptax, transferfee);
}

} /* namespace hku */
