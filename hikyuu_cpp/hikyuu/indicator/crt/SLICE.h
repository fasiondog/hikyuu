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
 * @param start 起始范围，可为负数
 * @param end 终止范围（不包含本身），可为负数
 * @ingroup Indicator
 */
Indicator HKU_API SLICE(const PriceList& data, int64_t start, int64_t end);

/**
 * 获取某指标中指定范围的数据
 * @param start 起始范围，可为负数
 * @param end 终止范围（不包含本身），可为负数
 * @param result_index 源数据中指定的结果集
 * @ingroup Indicator
 */
Indicator HKU_API SLICE(int64_t start, int64_t end, int result_index = 0);

/**
 * 获取某指标中指定范围的数据
 * @param ind 源数据
 * @param start 起始范围，可为负数
 * @param end 终止范围（不包含本身），可为负数
 * @param result_index 源数据中指定的结果集
 * @ingroup Indicator
 */
inline Indicator SLICE(const Indicator& ind, int64_t start, int64_t end, int result_index = 0) {
    return SLICE(start, end, result_index)(ind);
}

}  // namespace hku
