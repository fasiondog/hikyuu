/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#pragma once

#include "ScoresFilterBase.h"

namespace hku {

/**
 * @ingroup MultiFactor
 * @{
 */

ScoresFilterPtr HKU_API SCFilter_AmountLimit(double min_amount_percent_limit = 0.1);

ScoresFilterPtr HKU_API SCFilter_Group(int group, int group_index);

ScoresFilterPtr HKU_API SCFilter_Price(double min_price, double max_price);

/* @} */
}  // namespace hku