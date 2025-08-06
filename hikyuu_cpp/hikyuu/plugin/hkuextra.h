/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/Stock.h"

namespace hku {

/**
 * @brief 注册扩展 K 线类型
 * @param ktype 扩展 K 线类型
 * @param basetype 对应的基础 K 线类型
 * @param minutes 每根K线所包含的分钟数
 * @param getPhaseEnd 获取K线所属的阶段结束时间
 */
void HKU_API registerKTypeExtra(const string& ktype, const string& basetype, int32_t minutes,
                                std::function<Datetime(const Datetime&)> getPhaseEnd);

void HKU_API releaseKExtra();

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

}  // namespace hku