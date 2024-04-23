/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/db_connect/TableMacro.h"

namespace hku {

struct HistoryFinanceTable {
    TABLE_BIND4(HistoryFinanceTable, HistoryFinance, file_date, report_date, market_code, values)
    uint64_t file_date;
    uint64_t report_date;
    std::string market_code;
    // std::vector<float> values;
    std::vector<char> values;
};

}  // namespace hku