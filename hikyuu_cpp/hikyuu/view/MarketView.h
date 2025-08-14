/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#pragma once

#include <arrow/api.h>
#include "hikyuu/Stock.h"

namespace hku {

/**
 * 获取指定股票集合在指定交易日的行情数据，不包含当日停牌无数据的股票
 * @ingroup View
 * @param stks 股票列表
 * @param date 交易日
 * @param market 市场代码（用于获取交易日历）
 * @return
 */
std::shared_ptr<arrow::Table> HKU_API getMarketView(const StockList& stks, const Datetime& date,
                                                    const string& market = "SH");

/**
 * 获取指定股票集合在最后交易日的行情数据，不包含当日停牌无数据的股票。如自动接收行情数据，则为实时行情数据
 * @ingroup View
 * @param stks 股票列表
 * @param market 市场代码（用于获取交易日历）
 * @return
 */
std::shared_ptr<arrow::Table> HKU_API getMarketView(const StockList& stks,
                                                    const string& market = "SH");

}  // namespace hku
