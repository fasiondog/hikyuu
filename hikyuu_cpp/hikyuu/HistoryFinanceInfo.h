/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#pragma once

#include "DataType.h"

namespace hku {

struct HKU_API HistoryFinanceInfo {
    Datetime reportDate;
    vector<float> values;

    HistoryFinanceInfo() = default;
    HistoryFinanceInfo(const HistoryFinanceInfo&) = default;
    HistoryFinanceInfo(HistoryFinanceInfo&& rv)
    : reportDate(std::move(rv.reportDate)), values(std::move(rv.values)) {}

    HistoryFinanceInfo& operator=(const HistoryFinanceInfo&);
    HistoryFinanceInfo& operator=(HistoryFinanceInfo&&);
};

}  // namespace hku