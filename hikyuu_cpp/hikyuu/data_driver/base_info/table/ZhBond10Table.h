/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/db_connect/TableMacro.h"

namespace hku {

struct ZhBond10Table {
    TABLE_BIND2(ZhBond10Table, zh_bond10, date, value)
    int64_t date{0};
    int64_t value{0};
};

}  // namespace hku