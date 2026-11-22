/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-11-22
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_PLUGIN_MARKETCONFIGCHECK_H
#define HIKYUU_PLUGIN_MARKETCONFIGCHECK_H

#include "../MarketInfo.h"

namespace hku {

/**
 * Validate a market registration before it is written into the baseinfo: a configuration that
 * the MarketInfo constructor rejects would otherwise be dropped when the baseinfo is reloaded
 * (the market then disappears without a trace)
 * @return true when the registration may be written
 */
inline bool checkMarketConfig(const string& market, const string& name, const string& description,
                              const string& index_code, uint64_t open1, uint64_t close1,
                              uint64_t open2, uint64_t close2, int tzOffsetMin, int dstRule,
                              int dataLag) {
    if (market.empty()) {
        HKU_ERROR("Reject the registration of an empty market!");
        return false;
    }

    try {
        TimeDelta o1 = MarketInfo::hhmmToTimeDelta(open1);
        TimeDelta c1 = MarketInfo::hhmmToTimeDelta(close1);
        TimeDelta o2 = MarketInfo::hhmmToTimeDelta(open2);
        TimeDelta c2 = MarketInfo::hhmmToTimeDelta(close2);
        [[maybe_unused]] MarketInfo info(market, name, description, index_code, Datetime(), o1, c1,
                                         o2, c2, tzOffsetMin, dstRule, dataLag);
    } catch (const std::exception& e) {
        HKU_ERROR("Reject the market [{}] registration: {}", market, e.what());
        return false;
    }
    return true;
}

}  // namespace hku

#endif /* HIKYUU_PLUGIN_MARKETCONFIGCHECK_H */
