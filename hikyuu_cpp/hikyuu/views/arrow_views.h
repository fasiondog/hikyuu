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

//-----------------------------------------------------------
// arrow错误时抛出异常, 其他情况仅可能返回为空表, 而不是空指针
//-----------------------------------------------------------

/**
 * 获取指定股票集合在指定交易日的行情数据，不包含当日停牌无数据的股票
 * @param stks 股票列表
 * @param date 交易日
 * @param market 市场代码（用于获取交易日历）
 * @return
 */
std::shared_ptr<arrow::Table> HKU_API getMarketView(const StockList& stks, const Datetime& date,
                                                    const string& market = "SH");

/**
 * 获取指定股票集合在最后交易日的行情数据，不包含当日停牌无数据的股票。如自动接收行情数据，则为实时行情数据
 * @param stks 股票列表
 * @param market 市场代码（用于获取交易日历）
 * @return
 */
std::shared_ptr<arrow::Table> HKU_API getMarketView(const StockList& stks,
                                                    const string& market = "SH");

/**
 * 计算指定证券列表的各指标结果，以 arrow table 形式返回
 * @param stks 指定证券列表
 * @param inds 指定指标列表
 * @param query 查询条件
 * @param market 指定行情市场（用于日期对齐）
 * @return * arrow::Result<std::shared_ptr<arrow::Table>>
 */
std::shared_ptr<arrow::Table> HKU_API getIndicatorsView(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const KQuery& query,
                                                        const string& market = "SH");

/**
 * 获取指定日期的各指定证券的各指标视图
 * @param stks 证券列表
 * @param inds 指标列表
 * @param date 指定日期
 * @param cal_len 计算需要的数据长度
 * @param ktype k线类型
 * @param market 指定行情市场（用于日期对齐）
 * @return arrow::Result<std::shared_ptr<arrow::Table>>
 */
std::shared_ptr<arrow::Table> HKU_API getIndicatorsView(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const Datetime& date, size_t cal_len = 100,
                                                        const KQuery::KType& ktype = KQuery::DAY,
                                                        const string& market = "SH");

/**
 * Indicator转换为 Arrow Table，包含时间序列，则带有时间列，否则无时间列
 * @param ind Indicator
 * @return arrow::Result<std::shared_ptr<arrow::Table>>
 */
std::shared_ptr<arrow::Table> HKU_API getIndicatorView(const Indicator& ind);

/**
 * Indicator转换为 Arrow Table，仅包含值，不包含时间序列
 * @return arrow::Result<std::shared_ptr<arrow::Table>>
 */
std::shared_ptr<arrow::Table> HKU_API getIndicatorValueView(const Indicator& ind);

/**
 * KData 转换至 arrow table
 * @return arrow::Result<std::shared_ptr<arrow::Table>>
 */
std::shared_ptr<arrow::Table> HKU_API getKDataView(const KData& kdata);

/** KRecordList 转换至 arrow table */
std::shared_ptr<arrow::Table> HKU_API getKRecordListView(const KRecordList& ks);

/** TimeLineList 转换至 arrow table */
std::shared_ptr<arrow::Table> HKU_API getTimeLineListView(const TimeLineList& ts);

/** TransRecordList 转换至 arrow table */
std::shared_ptr<arrow::Table> HKU_API getTransRecordListView(const TransRecordList& ts);

/** StockWeightList 转换至 arrow table */
std::shared_ptr<arrow::Table> HKU_API getStockWeightListView(const StockWeightList& sws);

/** DatetimeList 转换至 arrow table */
std::shared_ptr<arrow::Table> HKU_API getDatetimeListView(const DatetimeList& dates);

/** TradeRecordList 转换至 arrow table */
std::shared_ptr<arrow::Table> HKU_API getTradeRecordListView(const TradeRecordList& trades);

/** PositionList 转换至 arrow table */
std::shared_ptr<arrow::Table> HKU_API
getPositionRecordListView(const PositionRecordList& positions);

/** @} */
}  // namespace hku
