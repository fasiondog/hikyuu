/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#pragma once

#include "arrow_common.h"
#include "hikyuu/Stock.h"
#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/trade_manage/PositionRecord.h"
#include "hikyuu/trade_manage/TradeRecord.h"

namespace hku {

/**
 * @ingroup Views
 * @{
 */

/**
 * 获取指定股票集合在指定交易日的行情数据，不包含当日停牌无数据的股票
 * @param stks 股票列表
 * @param date 交易日
 * @param market 市场代码（用于获取交易日历）
 * @return
 */
arrow::Result<std::shared_ptr<arrow::Table>> HKU_API getMarketView(const StockList& stks,
                                                                   const Datetime& date,
                                                                   const string& market = "SH");

/**
 * 获取指定股票集合在最后交易日的行情数据，不包含当日停牌无数据的股票。如自动接收行情数据，则为实时行情数据
 * @param stks 股票列表
 * @param market 市场代码（用于获取交易日历）
 * @return
 */
arrow::Result<std::shared_ptr<arrow::Table>> HKU_API getMarketView(const StockList& stks,
                                                                   const string& market = "SH");

/**
 * @brief 计算指定证券列表的各指标结果，以 arrow table 形式返回
 * @param stks 指定证券列表
 * @param inds 指定指标列表
 * @param query 查询条件
 * @param market 指定行情市场（用于日期对齐）
 * @return * arrow::Result<std::shared_ptr<arrow::Table>>
 */
[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getIndicatorsView(const StockList& stks, const IndicatorList& inds, const KQuery& query,
                  const string& market = "SH");

/**
 * @brief 获取指定日期的各指定证券的各指标视图
 * @param stks 证券列表
 * @param inds 指标列表
 * @param date 指定日期
 * @param cal_len 计算需要的数据长度
 * @param ktype k线类型
 * @param market 指定行情市场（用于日期对齐）
 * @return arrow::Result<std::shared_ptr<arrow::Table>>
 */
[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API getIndicatorsView(
  const StockList& stks, const IndicatorList& inds, const Datetime& date, size_t cal_len = 100,
  const KQuery::KType& ktype = KQuery::DAY, const string& market = "SH");

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getKRecordListView(const KData& kdata);

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getKRecordListView(const KRecordList& ks);

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getTimeLineListView(const TimeLineList& ts);

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getTransRecordListView(const TransRecordList& ts);

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getStockWeightListView(const StockWeightList& sws);

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getDatetimeListView(const DatetimeList& dates);

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getTradeRecordListView(const TradeRecordList& trades);

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> HKU_API
getPositionRecordListView(const PositionRecordList& positions);

/** @} */
}  // namespace hku
