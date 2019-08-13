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

#include "../../db_connect/TableMacro.h"

namespace hku {

class StockWeightTable {
    TABLE_BIND9(stkWeight, stockid, date, countAsGift, countForSell, priceForSell,
                bonus, countOfIncreasement, totalCount, freeCount) 

public:
    StockWeightTable()
    : m_id(0), stockid(0), date(0), countAsGift(0), countForSell(0),
      priceForSell(0), bonus(0), countOfIncreasement(0), totalCount(0), freeCount(0) {}

    uint64 stockid() const { return stockid; }
    uint64 date() const { return date; }
    double countAsGift() const { return countAsGift; }
    double countForSell() const { return countForSell; }
    double priceForSell() const { return priceForSell; }
    double bonus() const { return bonus; }
    double countOfIncreasement() const { return countOfIncreasement; }
    double totalCount() const { return totalCount; }
    double freeCount() const { return freeCount; }

private:
    uint64 stockid;
    uint64 date;
    double countAsGift;
    double countForSell;
    double priceForSell;
    double bonus;
    double countOfIncreasement;
    double totalCount;
    double freeCount;
};

} /* namespace */

#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKWEIGHTTABLE_H */