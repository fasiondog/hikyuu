/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#pragma once

#include "DataType.h"

namespace hku {

/**
 * Historical financial information record
 * @ingroup StockManage
 */
struct HKU_API HistoryFinanceInfo {
    Datetime fileDate;     ///< Used to distinguish the Q1, half-year, Q3 and annual reports
    Datetime reportDate;   ///< Financial report date
    vector<float> values;  ///< Detailed financial information; the field index can be queried with
                           ///< StockManager.getHistoryFinanceAllFields

    HistoryFinanceInfo() = default;
    HistoryFinanceInfo(const HistoryFinanceInfo&) = default;
    HistoryFinanceInfo(HistoryFinanceInfo&& rv) noexcept
    : fileDate(std::move(rv.fileDate)),
      reportDate(std::move(rv.reportDate)),
      values(std::move(rv.values)) {}

    HistoryFinanceInfo& operator=(const HistoryFinanceInfo&) noexcept;
    HistoryFinanceInfo& operator=(HistoryFinanceInfo&&) noexcept;
};

}  // namespace hku