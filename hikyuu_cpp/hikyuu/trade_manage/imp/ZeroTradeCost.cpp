/*
 * ZeroTradeCost.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "ZeroTradeCost.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ZeroTradeCost)
#endif

namespace hku {

ZeroTradeCost::ZeroTradeCost() : TradeCostBase("TC_Zero") {}

ZeroTradeCost::~ZeroTradeCost() {}

CostRecord ZeroTradeCost ::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num) const {
    return CostRecord();
}

CostRecord ZeroTradeCost ::getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                       double num) const {
    return CostRecord();
}

TradeCostPtr ZeroTradeCost::_clone() {
    return TradeCostPtr(new ZeroTradeCost());
}

TradeCostPtr HKU_API TC_Zero() {
    return TradeCostPtr(new ZeroTradeCost());
}

} /* namespace hku */
