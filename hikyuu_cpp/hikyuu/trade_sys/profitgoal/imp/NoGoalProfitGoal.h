/*
 * NoGoalProfitGoal.h
 *
 *  Created on: 2016年5月6日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_PROFITGOAL_IMP_NOGOALPROFITGOAL_H_
#define TRADE_SYS_PROFITGOAL_IMP_NOGOALPROFITGOAL_H_

#include "../ProfitGoalBase.h"

namespace hku {

class NoGoalProfitGoal : public ProfitGoalBase {
    PROFITGOAL_IMP(NoGoalProfitGoal)
    PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NoGoalProfitGoal();
    virtual ~NoGoalProfitGoal();
};

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_IMP_NOGOALPROFITGOAL_H_ */
