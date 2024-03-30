/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/multifactor/MultiFactorBase.h"
#include "../SelectorBase.h"

namespace hku {

SelectorPtr HKU_API SE_Multi_factor(const MFPtr& mf, int topn = 10);

SelectorPtr HKU_API SE_Multi_factor(const IndicatorList& src_inds, const StockList& stks,
                                    const KQuery& query, int topn = 10, int ic_n = 5,
                                    int ic_rolling_n = 120, const Stock& ref_stk = Stock(),
                                    const string& mode = "MF_ICIRWeight");

}  // namespace hku