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
 * @ingroup ProfitGoal
 */
ProfitGoalPtr HKU_API PG_FixedHoldDays(int days = 5);

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_CRT_PG_FIXEDPERCENT_H_ */
