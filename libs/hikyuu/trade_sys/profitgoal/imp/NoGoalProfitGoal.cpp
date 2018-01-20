/*
 * NoGoalProfitGoal.cpp
 *
 *  Created on: 2016年5月6日
 *      Author: Administrator
 */

#include "NoGoalProfitGoal.h"

namespace hku {

NoGoalProfitGoal::NoGoalProfitGoal() : ProfitGoalBase("PG_NoGoal"){

}

NoGoalProfitGoal::~NoGoalProfitGoal() {

}

void NoGoalProfitGoal::_calculate() {

}

price_t NoGoalProfitGoal::getGoal(const Datetime& datetime, price_t price) {
    return Null<price_t>();
}

ProfitGoalPtr HKU_API PG_NoGoal() {
    return ProfitGoalPtr(new NoGoalProfitGoal);
}

} /* namespace hku */
