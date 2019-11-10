/*
 * PG_NoGoal.h
 *
 *  Created on: 2016年5月6日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_PROFITGOAL_CRT_PG_NOGOAL_H_
#define TRADE_SYS_PROFITGOAL_CRT_PG_NOGOAL_H_

#include "../ProfitGoalBase.h"

namespace hku {

/**
 * 无盈利目标策略，通常为了进行测试或对比
 * @return PGPtr
 */
ProfitGoalPtr HKU_API PG_NoGoal();

} /* namespace hku */

#endif /* TRADE_SYS_PROFITGOAL_CRT_PG_NOGOAL_H_ */
