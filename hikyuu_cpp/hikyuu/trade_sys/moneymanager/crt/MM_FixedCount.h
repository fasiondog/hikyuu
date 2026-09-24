/*
 * MM_FixedCount.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef FIXEDCOUNT_MM_H_
#define FIXEDCOUNT_MM_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * Fixed trade quantity money management strategy
 * @details A fixed quantity is bought every time.
 * @param n the quantity bought every time (it should be an integer multiple of the minimum trade
 *          quantity of the trading object, the program does not check this here)
 * @note 1) This strategy is mainly used to test and compare the results with the other strategies,
 * it does not conform to the reality itself. \n 2) This strategy does not judge the existing
 * positions; if a trade cannot be made with the existing positions, that judgment should be the
 * responsibility of the System itself
 * @ingroup MoneyManager
 */
MoneyManagerPtr HKU_API MM_FixedCount(double n = 100);

}  // namespace hku

#endif /* FIXEDCOUNT_MM_H_ */
