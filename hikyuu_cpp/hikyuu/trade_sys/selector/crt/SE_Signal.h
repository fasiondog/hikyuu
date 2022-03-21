/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-19
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

/**
 * @brief 信号选择器，仅依靠系统买入信号进行选中
 * @return SEPtr
 * @ingroup Selector
 */
SEPtr HKU_API SE_Signal();

/**
 * @brief 信号选择器，仅依靠系统买入信号进行选中
 * @param stock_list 股票列表
 * @param sys 原型系统
 * @return SEPtr
 * @ingroup Selector
 */
SEPtr HKU_API SE_Signal(const StockList& stock_list, const SystemPtr& sys);

}  // namespace hku