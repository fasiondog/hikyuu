/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/Stock.h"

namespace hku {

void HKU_API registerKTypeExtra(const string& ktype, const string& basetype, int32_t minutes,
                                std::function<Datetime(const Datetime&)> getPhaseEnd);

void HKU_API releaseKExtra();

bool HKU_API isExtraKType(const string& ktype);

int32_t HKU_API getKTypeExtraMinutes(const string& ktype);

std::vector<string> HKU_API getExtraKTypeList();

KRecordList HKU_API getExtraKRecordList(const Stock& stk, const KQuery& query);

}  // namespace hku