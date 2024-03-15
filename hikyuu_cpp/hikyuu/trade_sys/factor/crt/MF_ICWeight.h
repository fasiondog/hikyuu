/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#pragma once

#include "../MultiFactorBase.h"

namespace hku {

MultiFactorPtr HKU_API MF_ICWeight(const IndicatorList& inds, const StockList& stks,
                                   const KQuery& query, const Stock& ref_stk, int ic_n = 5,
                                   int ic_rolling_n = 120);

}