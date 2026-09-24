/*
 * MM_FixedUnits.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDUNITS_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDUNITS_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * Fixed units money management strategy
 * Formula: buy quantity = current cash / n / current risk
 * @param n
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedUnits(int n = 33);

}  // namespace hku

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDUNITS_H_ */
