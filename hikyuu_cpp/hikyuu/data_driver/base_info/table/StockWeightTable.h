/*
 * StockWeightTable.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKWEIGHTTABLE_H
#define HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKWEIGHTTABLE_H

#include "../../../utilities/db_connect/TableMacro.h"

namespace hku {

class StockWeightTable {
    TABLE_BIND9(StockWeightTable, stkweight, stockid, date, countAsGift, countForSell, priceForSell,
                bonus, countOfIncreasement, totalCount, freeCount)

public:
    uint64_t stockid{0};
    uint64_t date{0};
    double countAsGift{0.};
    double countForSell{0.};
    double priceForSell{0.};
    double bonus{0.};
    double countOfIncreasement{0.};
    double totalCount{0.};
    double freeCount{0.};
};

struct StockWeightTableView {
    TABLE_BIND9(StockWeightTableView, stkweight, market_code, date, countAsGift, countForSell,
                priceForSell, bonus, countOfIncreasement, totalCount, freeCount)
    string market_code;
    uint64_t date{0};
    double countAsGift{0.};
    double countForSell{0.};
    double priceForSell{0.};
    double bonus{0.};
    double countOfIncreasement{0.};
    double totalCount{0.};
    double freeCount{0.};
};

}  // namespace hku

#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKWEIGHTTABLE_H */