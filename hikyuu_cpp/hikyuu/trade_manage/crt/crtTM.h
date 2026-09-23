/*
 * crtTM.h
 *
 *  Created on: 2013-2-26
 *      Author: fasiondog
 */

#pragma once
#ifndef CRTTM_H_
#define CRTTM_H_

#include "../TradeManager.h"
#include "TC_Zero.h"

namespace hku {

/**
 * Create the trade management module, it manages the trade records and the fund usage of the
 * account
 * @details The slippage considered needs the high or low price of the day, so it is not handled
 *          inside this module
 * @ingroup TradeManagerClass
 * @param datetime account creation date, 1990-1-1 by default
 * @param initcash initial cash, 100000 by default
 * @param costfunc trade cost algorithm, the zero cost algorithm by default
 * @param name account name, "SYS" by default
 * @see TradeManager
 */
TradeManagerPtr HKU_API crtTM(const Datetime& datetime = Datetime(199001010000LL),
                              price_t initcash = 100000.0, const TradeCostPtr& costfunc = TC_Zero(),
                              const string& name = "SYS");

}  // namespace hku

#endif /* CRTTM_H_ */
