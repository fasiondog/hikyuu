/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#pragma once

#include <hikyuu/config.h>

#if HKU_ENABLE_ARROW
#include <arrow/api.h>

namespace hku {

//-------------------------------
// Views 扩展
//-------------------------------

std::shared_ptr<arrow::Table> getIndicatorsViewParallel(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const KQuery& query, const string& market);

std::shared_ptr<arrow::Table> getIndicatorsViewParallel(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const Datetime& date, size_t cal_len,
                                                        const KQuery::KType& ktype,
                                                        const string& market);

std::shared_ptr<arrow::Table> getMarketViewParallel(const StockList& stks, const Datetime& date,
                                                    const string& market);

}  // namespace hku

#endif  // HKU_ENABLE_ARROW
