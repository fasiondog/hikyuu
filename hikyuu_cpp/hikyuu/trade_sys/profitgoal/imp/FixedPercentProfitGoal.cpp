/*
 * FixedPercentProfitGoal.cpp
 *
 *  Created on: 2016年5月6日
 *      Author: Administrator
 */

#include "FixedPercentProfitGoal.h"

namespace hku {

FixedPercentProfitGoal::FixedPercentProfitGoal()
: ProfitGoalBase("PG_FixedPercent") {
    setParam<double>("p", 0.2);
}

FixedPercentProfitGoal::~FixedPercentProfitGoal() {

}

void FixedPercentProfitGoal::_calculate() {

}

price_t FixedPercentProfitGoal::getGoal(const Datetime& datetime, price_t price) {
    Stock stock = getTO().getStock();
    PositionRecord position = getTM()->getPosition(stock);
    price_t result = Null<price_t>();
    if (position.number != 0) {
        price_t per_price = position.buyMoney / position.number;
        result = per_price * (1 + getParam<double>("p"));
    } else {
        result = price * (1 + getParam<double>("p"));
    }

    return result;
    //return price * (1 + getParam<double>("p"));
}

ProfitGoalPtr HKU_API PG_FixedPercent(double p) {
    ProfitGoalPtr ptr = make_shared<FixedPercentProfitGoal>();
    ptr->setParam<double>("p", p);
    return ptr;
}

} /* namespace hku */
