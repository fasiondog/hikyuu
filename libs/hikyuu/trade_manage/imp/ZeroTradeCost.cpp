/*
 * ZeroTradeCost.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "ZeroTradeCost.h"

namespace hku {

ZeroTradeCost::ZeroTradeCost(): TradeCostBase("ZeroTradeCost") {

}

ZeroTradeCost::~ZeroTradeCost() {

}

CostRecord ZeroTradeCost
::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price, size_t num) const {
    return CostRecord();
}


CostRecord ZeroTradeCost
::getSellCost(const Datetime& datetime, const Stock& stock, price_t price, size_t num) const {
    return CostRecord();
}

TradeCostPtr ZeroTradeCost::_clone() {
    return TradeCostPtr(new ZeroTradeCost());
}

} /* namespace hku */
