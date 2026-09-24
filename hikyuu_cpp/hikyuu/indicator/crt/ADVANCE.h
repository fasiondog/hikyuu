/*
 * ADVANCE.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_ADVANCE_H_
#define INDICATOR_CRT_ADVANCE_H_

#include "../Indicator.h"
#include "../../StockTypeInfo.h"

namespace hku {

/**
 * Number of the rising stocks. When there is a given context and ignore_context is false, the
 * query, market and stk_type parameters are ignored.
 * @param query query condition
 * @param market the market it belongs to; when it is "" all the markets are got
 * @param stk_type security type; when it is greater than constant.STOCKTYPE_TMP all the security
 *                 types are got
 * @param ignore_context whether to ignore the context. When it is ignored, the query, market and
 *                       stk_type parameters are used forcibly.
 * @param fill_null fill the missing date data with nan when the data is aligned.
 * @ingroup Indicator
 */
Indicator HKU_API ADVANCE(const KQuery& query = KQueryByIndex(-100), const string& market = "SH",
                          int stk_type = STOCKTYPE_A, bool ignore_context = false,
                          bool fill_null = true);

}  // namespace hku

#endif /* INDICATOR_CRT_ADVANCE_H_ */
