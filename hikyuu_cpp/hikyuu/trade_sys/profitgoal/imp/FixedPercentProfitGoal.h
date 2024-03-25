/*
 * FixedPercentProfitGoal.h
 *
 *  Created on: 2016年5月6日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_PROFITGOAL_IMP_FIXEDPERCENTPROFITGOAL_H_
#define TRADE_SYS_PROFITGOAL_IMP_FIXEDPERCENTPROFITGOAL_H_

#include "../ProfitGoalBase.h"

namespace hku {

class FixedPercentProfitGoal : public ProfitGoalBase {
    PROFITGOAL_IMP(FixedPercentProfitGoal)
    PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedPercentProfitGoal();
    virtual ~FixedPercentProfitGoal();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_IMP_FIXEDPERCENTPROFITGOAL_H_ */
