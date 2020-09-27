/*
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2020-9-27
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DATA_DRIVER_KDATA_MYSQL_KRECORDTABLE_H
#define HIKYUU_DATA_DRIVER_KDATA_MYSQL_KRECORDTABLE_H

#include "../../../utilities/db_connect/SQLStatementBase.h"

namespace hku {

class KRecordTable {
public:
    KRecordTable();

private:
    int64_t date;
    price_t open;
    price_t high;
    price_t low;
    price_t close;
    price_t amount;
    price_t count;
};

}  // namespace hku

#endif /* HIKYUU_DATA_DRIVER_KDATA_MYSQL_KRECORDTABLE_H */
