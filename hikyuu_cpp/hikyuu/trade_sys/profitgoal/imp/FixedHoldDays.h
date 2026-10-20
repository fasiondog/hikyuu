/*
 * FixedHoldDays.h
 *
 *  Created on: 2018-1-20
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_PROFITGOAL_IMP_FIXEDHOLDDAYS_H_
#define TRADE_SYS_PROFITGOAL_IMP_FIXEDHOLDDAYS_H_

#include "../ProfitGoalBase.h"

#include <vector>

namespace hku {

class FixedHoldDays : public ProfitGoalBase {
    PROFITGOAL_IMP(FixedHoldDays)
    PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedHoldDays();
    virtual ~FixedHoldDays();

    virtual void _checkParam(const string& name) const override;
    virtual void _reset() override;
    virtual void _calculate() override;
    virtual price_t _getShortGoal(const Datetime& datetime, price_t price) override;

private:
    /** Number of traded days in [take_date, datetime) reached the hold-days threshold */
    bool _shouldExit(const Datetime& datetime, const Datetime& take_date);

    std::vector<Datetime> m_trading_days;
};

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_IMP_FIXEDHOLDDAYS_H_ */
