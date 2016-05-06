/*
 * NoGoalProfitGoal.cpp
 *
 *  Created on: 2016年5月6日
 *      Author: Administrator
 */

#include "NoGoalProfitGoal.h"

namespace hku {

NoGoalProfitGoal::NoGoalProfitGoal() : ProfitGoalBase("NoGoal"){

}

NoGoalProfitGoal::~NoGoalProfitGoal() {

}

void NoGoalProfitGoal::_calculate() {

}

price_t NoGoalProfitGoal::getGoal(const Datetime& datetime, price_t price) {
    return 0.0;
}

ProfitGoalPtr HKU_API PG_NoGoal() {
    return ProfitGoalPtr(new NoGoalProfitGoal);
}

} /* namespace hku */
