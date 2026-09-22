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
 * @brief Signal selector, it selects with the system buy signals only
 * @return SEPtr
 * @ingroup Selector
 */
SEPtr HKU_API SE_Signal();

/**
 * @brief Signal selector, it selects with the system buy signals only
 * @param stock_list stock list
 * @param sys prototype system
 * @return SEPtr
 * @ingroup Selector
 */
SEPtr HKU_API SE_Signal(const StockList& stock_list, const SystemPtr& sys);

}  // namespace hku