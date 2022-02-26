/*
 * SE_Fixed.h
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_
#define TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_

#include "../SelectorBase.h"

namespace hku {

/**
 * @brief 固定选择器，每天都选择指定的交易系统
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_Fixed();

/**
 * @brief 固定选择器，每天都选择指定的交易系统
 * @details 对指定的每个股票以原型系统创建相应的交易系统
 * @param stock_list 指定的股票列表
 * @param sys 原型系统
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_Fixed(const StockList& stock_list, const SystemPtr& sys);

} /* namespace hku */

#endif /* TRADE_SYS_SELECTOR_CRT_SE_FIXED_H_ */
