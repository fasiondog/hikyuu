/*
 * FixedAPre2015TradeCost.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "FixedAPre2015TradeCost.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedAPre2015TradeCost)
#endif

namespace hku {

FixedAPre2015TradeCost::FixedAPre2015TradeCost() : FixedATradeCostBase("TC_FixedAPre2015") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.001);
    setParam<price_t>("lowest_transferfee", 1.0);
}

FixedAPre2015TradeCost::FixedAPre2015TradeCost(price_t commission, price_t lowestCommission,
                                               price_t stamptax, price_t transferfee,
                                               price_t lowestTransferfee)
: FixedATradeCostBase("FixedAPre2015TradeCost") {
    setParam<price_t>("commission", commission);
    setParam<price_t>("lowest_commission", lowestCommission);
    setParam<price_t>("stamptax", stamptax);
    setParam<price_t>("transferfee", transferfee);
    setParam<price_t>("lowest_transferfee", lowestTransferfee);
}

FixedAPre2015TradeCost::~FixedAPre2015TradeCost() {}

void FixedAPre2015TradeCost::_checkParam(const string& name) const {
    if ("lowest_transferfee" == name) {
        HKU_ASSERT(getParam<price_t>("lowest_transferfee") >= 0.0);
    } else {
        FixedATradeCostBase::_checkParam(name);
    }
}

price_t FixedAPre2015TradeCost::_calcTransferFee(const Stock& stock, price_t, double num) const {
    // The Shanghai Stock Exchange charges a transfer fee by traded quantity, with a minimum value
    if (stock.market() != "SH") {
        return 0.0;
    }
    return num > 1000 ? roundEx(getParam<price_t>("transferfee") * num, stock.precision())
                      : getParam<price_t>("lowest_transferfee");
}

TradeCostPtr FixedAPre2015TradeCost::_clone() {
    return make_shared<FixedAPre2015TradeCost>();
}

TradeCostPtr HKU_API TC_FixedAPre2015(price_t commission, price_t lowestCommission,
                                      price_t stamptax, price_t transferfee,
                                      price_t lowestTransferfee) {
    return make_shared<FixedAPre2015TradeCost>(commission, lowestCommission, stamptax, transferfee,
                                               lowestTransferfee);
}

} /* namespace hku */
