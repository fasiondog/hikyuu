/*
 * PG_FixedPercent.h
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_PROFITGOAL_CRT_PG_FIXEDHOLDDAYS_H_
#define TRADE_SYS_PROFITGOAL_CRT_PG_FIXEDHOLDDAYS_H_

#include "../ProfitGoalBase.h"

namespace hku {

/**
 * Profit goal strategy of a fixed holding days
 * @param days: the allowed holding days (counted in trading days), 5 days by default
 * @return PGPtr
 * @details Once the position (long or short) has been held for at least days trading days,
 * the rule forces an exit on the next bar regardless of price. setTO must be called first,
 * otherwise there is no trading-day list and the rule never takes effect. The forced exit is
 * signalled through the goal value (0 for long, the current price for short), so the goalPrice
 * recorded for such an exit is not a meaningful target price
 * @ingroup ProfitGoal
 */
ProfitGoalPtr HKU_API PG_FixedHoldDays(int days = 5);

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_CRT_PG_FIXEDPERCENT_H_ */
