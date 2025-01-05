/*
 * DECLINE.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-3
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_DECLINE_H_
#define INDICATOR_CRT_DECLINE_H_

#include "../Indicator.h"
#include "../../StockTypeInfo.h"

namespace hku {

/**
 * 下跌家数。当存在指定上下文且 ignore_context 为 false 时，将忽略 query, market, stk_type 参数。
 * @param query 查询条件
 * @param market 所属市场，等于 "" 时，获取所有市场
 * @param stk_type 证券类型, 大于 constant.STOCKTYPE_TMP 时，获取所有类型证券
 * @param ignore_context 是否忽略上下文。忽略时，强制使用 query, market, stk_type 参数。
 * @param fill_null 日期对齐时，缺失数据填充 nan 值
 * @ingroup Indicator
 */
Indicator HKU_API DECLINE(const KQuery& query = KQueryByIndex(-100), const string& market = "SH",
                          int stk_type = STOCKTYPE_A, bool ignore_context = false,
                          bool fill_null = true);

}  // namespace hku

#endif /* INDICATOR_CRT_DECLINE_H_ */
