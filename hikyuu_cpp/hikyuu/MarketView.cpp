/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-13
 *      Author: fasiondog
 */

#include "MarketView.h"

namespace hku {

string MarketViewRecord::str() const {
    return fmt::format("{}, {}, {}, {}, {}, {}, {}, {}", stock.market_code(), stock.name(),
                       krecord.datetime.ymdhm(), krecord.openPrice, krecord.highPrice,
                       krecord.lowPrice, krecord.closePrice, krecord.transAmount,
                       krecord.transCount);
}

MarketView HKU_API getMarketView(const StockList& stks, const Datetime& datetime,
                                 const KQuery::KType& ktype) {
    MarketView ret;
    for (const auto& stk : stks) {
        if (!stk.isNull()) {
            auto krecord = stk.getKRecord(datetime, ktype);
            if (krecord.isValid()) {
                ret.emplace_back(stk, krecord);
            }
        }
    }
    return ret;
}

}  // namespace hku