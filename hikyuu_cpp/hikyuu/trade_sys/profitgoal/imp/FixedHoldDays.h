/*
 * FixedHoldDays.h
 *
 *  Created on: 2018年1月20日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_PROFITGOAL_IMP_FIXEDHOLDDAYS_H_
#define TRADE_SYS_PROFITGOAL_IMP_FIXEDHOLDDAYS_H_

#include "../ProfitGoalBase.h"

namespace hku {

class FixedHoldDays : public ProfitGoalBase {
    PROFITGOAL_IMP(FixedHoldDays)
    PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedHoldDays();
    virtual ~FixedHoldDays();

    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_IMP_FIXEDHOLDDAYS_H_ */
