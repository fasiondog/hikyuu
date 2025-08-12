/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#pragma once

#include "Stock.h"

namespace hku {

struct HKU_API MarketViewRecord {
    Stock stock;
    KRecord krecord;

    MarketViewRecord(const Stock& stock_, const KRecord& krecord_)
    : stock(stock_), krecord(krecord_) {}

    string str() const;
};

using MarketView = vector<MarketViewRecord>;

MarketView HKU_API getMarketView(const StockList& stks, const Datetime& datetime,
                                 const KQuery::KType& ktype = KQuery::DAY);

}  // namespace hku
