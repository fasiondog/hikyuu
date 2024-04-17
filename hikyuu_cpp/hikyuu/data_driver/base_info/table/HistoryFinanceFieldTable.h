/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/db_connect/TableMacro.h"

namespace hku {

struct HistoryFinanceFieldTable {
    TABLE_BIND1(HistoryFinanceFieldTable, HistoryFinanceField, name)
    std::string name;
};

}