/*
 * MM_FixedUnits.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDUNITS_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDUNITS_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * 固定单位资金管理策略
 * 公式: 买入数量 = 当前现金 / n / 当前风险risk
 * @param n
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedUnits(int n = 33);

}  // namespace hku

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDUNITS_H_ */
