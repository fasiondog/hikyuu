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
    return price * (1 + getParam<double>("p"));
}

ProfitGoalPtr HKU_API PG_FixedPercent(double p) {
    ProfitGoalPtr ptr = make_shared<FixedPercentProfitGoal>();
    ptr->setParam<double>("p", p);
    return ptr;
}

} /* namespace hku */
