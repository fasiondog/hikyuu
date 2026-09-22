/*
 * PG_FixedPercent.h
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_PROFITGOAL_CRT_PG_FIXEDPERCENT_H_
#define TRADE_SYS_PROFITGOAL_CRT_PG_FIXEDPERCENT_H_

#include "../ProfitGoalBase.h"

namespace hku {

/**
 * Fixed percentage profit goal, target price = buy price * (1 + p)
 * @param p percentage
 * @return PGPtr
 */
ProfitGoalPtr HKU_API PG_FixedPercent(double p = 0.2);

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_CRT_PG_FIXEDPERCENT_H_ */
