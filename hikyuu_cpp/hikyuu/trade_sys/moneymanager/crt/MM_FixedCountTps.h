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
 * @param buy_counts the buy quantities in turn; each entry must be finite and strictly greater
 *                   than 0 and the list must not be empty (use a shorter list to trade fewer
 *                   tiers)
 * @param sell_counts the sell quantities in turn; each entry must be finite and strictly greater
 *                    than 0 (use a shorter list to trade fewer tiers); an empty list makes the
 *                    first sell signal liquidate the whole position
 * @note A breached stop-loss (the per-share risk not greater than 0), an exhausted sell_counts
 *       list or a sell quantity above the current position (e.g. after an ex-rights adjustment)
 *       sells the whole remaining position
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HKU_API MM_FixedCountTps(const vector<double>& buy_counts,
                                         const vector<double>& sell_counts);

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDCOUNT_TPS_H_ */
