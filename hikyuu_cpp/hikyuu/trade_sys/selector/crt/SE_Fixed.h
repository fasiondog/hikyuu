/*
 * SE_Fixed.h
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_
#define TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_

#include "../SelectorBase.h"

namespace hku {

SelectorPtr HKU_API SE_Fixed();

SelectorPtr HKU_API SE_Fixed(const StockList& stock_list, const SystemPtr& sys);

} /* namespace hku */

#endif /* TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_ */
