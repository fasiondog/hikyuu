/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#pragma once

#include <arrow/api.h>
#include "hikyuu/Stock.h"
#include "hikyuu/indicator/Indicator.h"

namespace hku {

/**
 * @brief 注册扩展 K 线类型, 按时间合成
 * @param ktype 扩展 K 线类型
 * @param basetype 对应的基础 K 线类型
 * @param minutes 每根K线所包含的分钟数
 * @param getPhaseEnd 获取K线所属的阶段结束时间
 */
void HKU_API registerExtraKType(const string& ktype, const string& basetype, int32_t minutes,
                                std::function<Datetime(const Datetime&)> getPhaseEnd);

/**
 * 注册K线扩展类型，按 bar 数量合成
 * @param ktype 扩展 K 线类型
 * @param basetype 对应的基础 K 线类型
 * @param nbars 每根K线所包含的基础K线数
 */
void HKU_API registerExtraKType(const string& ktype, const string& basetype, int32_t nbars);

/**
 * 释放扩展K线
 * @note 某些情况下，注册扩展K线时，使用了 python 定义的周期结束日期转换函数，退出时可能引起 python
 * GIL 报错，需手工提前释放扩展K线
 */
void HKU_API releaseExtraKType();

//-------------------------------
// 以下函数为内部使用，无需引出
//-------------------------------

bool isExtraKType(const string& ktype);

int32_t getKTypeExtraMinutes(const string& ktype);

std::vector<string> getExtraKTypeList();

KRecordList getExtraKRecordList(const Stock& stk, const KQuery& query);

size_t getStockExtraCount(const Stock& stk, const string& ktype);

bool getStockExtraIndexRange(const Stock& stk, const KQuery& query, size_t& out_start,
                             size_t& out_end);

KDataImpPtr getKDataImp(const Stock& stk, const KQuery& query);

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