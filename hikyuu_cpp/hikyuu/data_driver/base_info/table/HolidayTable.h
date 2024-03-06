/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-06
 *     Author: fasiondog
 */

#pragma once

#include <limits>
#include "../../../utilities/db_connect/SQLStatementBase.h"
#include "../../../utilities/db_connect/TableMacro.h"

namespace hku {

class HolidayTable {
    TABLE_BIND1(HolidayTable, holiday, date)

public:
    Datetime datetime() const {
        HKU_CHECK(date <= 99999999, "Invalid holiday date: {}!", date);
        return Datetime(date * 10000LL);
    }

private:
    uint64_t date{Datetime().number()};
};

}  // namespace hku