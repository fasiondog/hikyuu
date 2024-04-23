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
 * 历史财务信息记录
 * @ingroup StockManage
 */
struct HKU_API HistoryFinanceInfo {
    Datetime fileDate;    ///< 用于区分一季报、半年报、三季报、年报
    Datetime reportDate;  ///< 财务报告日期
    vector<float>
      values;  ///< 详细财务信息，字段索引可使用 StockManager.getHistoryFinanceAllFields 查询

    HistoryFinanceInfo() = default;
    HistoryFinanceInfo(const HistoryFinanceInfo&) = default;
    HistoryFinanceInfo(HistoryFinanceInfo&& rv)
    : fileDate(std::move(rv.fileDate)),
      reportDate(std::move(rv.reportDate)),
      values(std::move(rv.values)) {}

    HistoryFinanceInfo& operator=(const HistoryFinanceInfo&);
    HistoryFinanceInfo& operator=(HistoryFinanceInfo&&);
};

}  // namespace hku