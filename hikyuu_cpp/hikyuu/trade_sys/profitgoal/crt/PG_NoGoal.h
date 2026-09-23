/*
 * PG_NoGoal.h
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_PROFITGOAL_CRT_PG_NOGOAL_H_
#define TRADE_SYS_PROFITGOAL_CRT_PG_NOGOAL_H_

#include "../ProfitGoalBase.h"

namespace hku {

/**
 * No profit goal strategy, it is usually used for the testing or the comparison
 * @return PGPtr
 */
ProfitGoalPtr HKU_API PG_NoGoal();

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_CRT_PG_NOGOAL_H_ */
