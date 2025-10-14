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
 * 过滤掉成交金额在评分列表末尾百分比范围内的截面
 * @note
 * 和传入的截面评分列表顺序相关，如果是降序，过滤的是成交金额较小的系统评分记录；反之，则是金额较大的系统评分记录
 * @param min_amount_percent_limit 最小成交金额百分比限制
 * @return ScoresFilterPtr
 */
ScoresFilterPtr HKU_API SCFilter_AmountLimit(double min_amount_percent_limit = 0.1);

/**
 * 筛选指定分组的截面
 * @param group 分组数量
 * @param group_index 分组索引，从0开始
 * @return ScoresFilterPtr
 */
ScoresFilterPtr HKU_API SCFilter_Group(int group = 10, int group_index = 0);

/**
 * 筛选指定价格范围 [min_price, max_price] 内的截面
 * @param min_price 最低价格
 * @param max_price 最高价格
 * @return ScoresFilterPtr
 */
ScoresFilterPtr HKU_API SCFilter_Price(double min_price = 10., double max_price = 100000.);

/* @} */
}  // namespace hku