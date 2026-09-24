/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/Stock.h"
#include "hikyuu/indicator/Indicator.h"

namespace hku {

/**
 * @brief Register an extended K-line type, synthesized by the time
 * @param ktype extended K-line type
 * @param basetype the corresponding base K-line type
 * @param minutes the number of the minutes contained in every K-line
 * @param getPhaseEnd get the phase end time to which the K-line belongs
 */
void HKU_API registerExtraKType(const string& ktype, const string& basetype, int32_t minutes,
                                std::function<Datetime(const Datetime&)> getPhaseEnd);

/**
 * Register an extended K-line type, synthesized by the number of the bars
 * @param ktype extended K-line type
 * @param basetype the corresponding base K-line type
 * @param nbars the number of the base K-lines contained in every K-line
 */
void HKU_API registerExtraKType(const string& ktype, const string& basetype, int32_t nbars);

/**
 * Release the extended K-line
 * @note In some cases, when the extended K-line is registered, a python-defined phase end date
 *       conversion function is used, which may cause a python
 * GIL error on exit; the extended K-line needs to be released manually in advance
 */
void HKU_API releaseExtraKType();

void HKU_API enableKDataCache(bool enable);

//-------------------------------
// The following functions are for internal use, they do not need to be exposed
//-------------------------------

bool isExtraKType(const string& ktype);

int32_t getKTypeExtraMinutes(const string& ktype);

std::vector<string> getExtraKTypeList();

KRecordList getExtraKRecordList(const Stock& stk, const KQuery& query);

size_t getStockExtraCount(const Stock& stk, const string& ktype);

bool getStockExtraIndexRange(const Stock& stk, const KQuery& query, size_t& out_start,
                             size_t& out_end);

KDataImpPtr getKDataImp(const Stock& stk, const KQuery& query);

bool canLazyLoad(const KQuery::KType& ktype);

}  // namespace hku