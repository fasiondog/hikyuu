/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * 获取 PriceList 中指定范围 [start, end) 的数据
 * @param data 源数据
 * @param start 起始范围
 * @param end 终止范围（不包含本身）
 * @ingroup Indicator
 */
Indicator HKU_API RANGE(const PriceList& data, int64_t start, int64_t end);

/**
 * 获取某指标中指定范围的数据
 * @param ind 源数据
 * @param start 起始范围
 * @param end 终止范围（不包含本身）
 * @param result_index 源数据中指定的结果集
 * @ingroup Indicator
 */
Indicator HKU_API RANGE(const Indicator& ind, int64_t start, int64_t end, int result_index = 0);

/**
 * 获取某指标中指定范围的数据
 * @param start 起始范围
 * @param end 终止范围（不包含本身）
 * @param result_index 源数据中指定的结果集
 * @ingroup Indicator
 */
Indicator HKU_API RANGE(int64_t start, int64_t end, int result_index = 0);

}  // namespace hku
