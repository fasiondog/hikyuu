/*
 * MM_FixedPercent.h
 *
 *  Created on: 2015-4-5
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDPERCENT_H_
#define TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDPERCENT_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * Percentage risk model
 * @details See "Financial Freedom Through Electronic Day Trading" (June 2008, China Machine Press)
 *          by Van K. Tharp, P312
 *          Formula: P (position size) = C (total risk) / R (risk per share) [here C, the cash, is
 * the total risk]
 * @param p the percentage of the total risk of every trade in the total assets, e.g. 0.02 means 2%
 *          of the total assets
 * @ingroup MoneyManager
 */
MoneyManagerPtr HKU_API MM_FixedPercent(double p);

}  // namespace hku

#endif /* TRADE_SYS_MONEYMANAGER_CRT_MM_FIXEDPERCENT_H_ */
