/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-14
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

/**
 * Selector that optimizes the maximum account assets
 * @return SEPtr
 */
SEPtr HKU_API SE_MaxFundsOptimal();

/**
 * Selector that performs the optimization with the Performance statistics results
 * @return SEPtr
 */
SEPtr HKU_API SE_PerformanceOptimal(const string& key = "Account Avg Annual Return %",
                                    int mode = 0);

/**
 * Selector that performs the optimization with a custom evaluation function
 * @param evaluate
 * @return SEPtr
 */
SEPtr HKU_API
SE_EvaluateOptimal(std::function<double(const SystemPtr&, const Datetime&)>&& evaluate);

}  // namespace hku