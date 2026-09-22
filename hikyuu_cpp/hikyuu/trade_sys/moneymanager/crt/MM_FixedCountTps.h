/*
 * MM_FixedCapital.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCOUNT_TPS_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCOUNT_TPS_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * @brief Money management strategy of buying / selling a fixed quantity consecutively.
 * @param buy_counts the buy quantities in turn
 * @param sell_counts the sell quantities in turn
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedCountTps(const vector<double>& buy_counts,
                                         const vector<double>& sell_counts);

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCOUNT_TPS_H_ */
