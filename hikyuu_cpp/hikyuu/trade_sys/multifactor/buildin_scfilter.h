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

ScoresFilterPtr HKU_API SCFilter_IgnoreNan();

ScoresFilterPtr HKU_API SCFilter_LessOrEqualValue(double value = 0.0);

ScoresFilterPtr HKU_API SCFilter_TopN(int topn = 10);

/**
 * Filter out the cross-sections whose turnover amount is within the percentage range at the end of
 * the score list
 * @note
 * It is related to the order of the passed cross-section score list: if it is in the descending
 * order, the system score records with a smaller turnover amount are filtered; otherwise the ones
 * with a larger amount are filtered
 * @param min_amount_percent_limit the minimum turnover amount percentage limit
 * @return ScoresFilterPtr
 */
ScoresFilterPtr HKU_API SCFilter_AmountLimit(double min_amount_percent_limit = 0.1);

/**
 * Filter the cross-sections of the given group
 * @param group number of the groups
 * @param group_index group index, starting from 0
 * @return ScoresFilterPtr
 */
ScoresFilterPtr HKU_API SCFilter_Group(int group = 10, int group_index = 0);

/**
 * Filter the cross-sections within the given price range [min_price, max_price]
 * @param min_price minimum price
 * @param max_price maximum price
 * @return ScoresFilterPtr
 */
ScoresFilterPtr HKU_API SCFilter_Price(double min_price = 10., double max_price = 100000.);

/* @} */
}  // namespace hku