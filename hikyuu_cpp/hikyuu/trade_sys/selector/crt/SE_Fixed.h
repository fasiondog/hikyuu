/*
 * SE_Fixed.h
 *
 *  Created on: 2018-1-13
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_
#define TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_

#include "../SelectorBase.h"

namespace hku {

/**
 * @brief Fixed selector, it selects the given trading system every day
 * @param weight fixed weight
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_Fixed(double weight = 1.0);

/**
 * @brief Fixed selector, it selects the given trading system every day
 * @details It creates the corresponding trading system with the prototype system for every given
 *          stock
 * @param stock_list the given stock list
 * @param sys prototype system
 * @param weight fixed weight
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_Fixed(const StockList& stock_list, const SystemPtr& sys,
                             double weight = 1.0);

} /* namespace hku */

#endif /* TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_ */
