/*
 * test_KDataTimeNormalize.cpp
 *
 *  Created on: 2026-11-07
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <algorithm>
#include <hikyuu/KData.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/Log.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_KDataTimeNormalize test_hikyuu_KDataTimeNormalize
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/**
 * End to end through the real hdf5 driver plus the Stock-layer normalization
 * @details Carrier: test_data/nasdaq_5min.h5 (synthesized, 2016-10-24~11-10 spanning the US
 * daylight end 2016-11-06, New York local wall clock; NASDAQ is -300/1 in the base database)
 */
TEST_CASE("test_KData_timeNormalize_h5") {
    StockManager& sm = StockManager::instance();
    Stock ndx = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!ndx.isNull());

    /** @arg The min15 index table: 364 buckets, the first local 09:45 becomes Beijing 21:45 */
    auto ks = ndx.getKRecordList(
      KQueryByDate(Datetime(201610010000LL), Datetime(201612310000LL), KQuery::MIN15));
    REQUIRE_EQ(ks.size(), (size_t)364);
    CHECK_EQ(ks.front().datetime, Datetime(201610242145LL));

    /** @arg Across the daylight end the same clock 09:45 maps to 21:45 (11-04) and 22:45 (11-07) */
    Datetime dst_first(201611042145LL);  // daylight (+12h)
    Datetime std_first(201611072245LL);  // standard (+13h)
    auto has_bar = [&](const Datetime& d) {
        return std::find_if(ks.begin(), ks.end(),
                            [&](const KRecord& k) { return k.datetime == d; }) != ks.end();
    };
    CHECK_UNARY(has_bar(dst_first));
    CHECK_UNARY(has_bar(std_first));

    /** @arg Ordering survives the conversion: local close 16:00 lands on the next Beijing day */
    auto unordered = std::adjacent_find(
      ks.begin(), ks.end(),
      [](const KRecord& a, const KRecord& b) { return !(a.datetime < b.datetime); });
    CHECK_UNARY(unordered == ks.end());
    /** @arg The last bucket: local 11-10 16:00 in the standard period is Beijing 11-11 05:00 */
    CHECK_EQ(ks.back().datetime, Datetime(201611110500LL));

    /** @arg getPos equals the index over the whole range (query and buffer share one expression) */
    KData kdata =
      ndx.getKData(KQueryByDate(Datetime(201610010000LL), Datetime(201612310000LL), KQuery::MIN15));
    REQUIRE_EQ(kdata.size(), (size_t)364);
    for (size_t i = 0; i < kdata.size(); i++) {
        REQUIRE_EQ(kdata.getPos(kdata[i].datetime), i);
    }

    /** @arg The min5 data group converts too: first 09:35 is Beijing 21:35, 1092 bars */
    auto min5 = ndx.getKRecordList(
      KQueryByDate(Datetime(201610010000LL), Datetime(201612310000LL), KQuery::MIN5));
    REQUIRE_EQ(min5.size(), (size_t)1092);
    CHECK_EQ(min5.front().datetime, Datetime(201610242135LL));
    auto unordered5 = std::adjacent_find(
      min5.begin(), min5.end(),
      [](const KRecord& a, const KRecord& b) { return !(a.datetime < b.datetime); });
    CHECK_UNARY(unordered5 == min5.end());

    /** @arg A local-expression range via the public helper selects the same bars as the unified */
    Datetime local_start(201611040945LL), local_end(201611080000LL);
    Datetime unified_start = sm.marketLocalToUnified(local_start, "NASDAQ");
    Datetime unified_end = sm.marketLocalToUnified(local_end, "NASDAQ");
    CHECK_EQ(unified_start, dst_first);
    auto part = ndx.getKRecordList(KQueryByDate(unified_start, unified_end, KQuery::MIN15));
    REQUIRE_UNARY(part.size() > 1);
    CHECK_EQ(part.front().datetime, unified_start);

    /** @arg getIndexRange in the unified writing selects exactly the same bars: the returned
     * indices are the driver-local ones and need no conversion */
    size_t range_start = 0, range_end = 0;
    REQUIRE_UNARY(ndx.getIndexRange(KQueryByDate(unified_start, unified_end, KQuery::MIN15),
                                    range_start, range_end));
    REQUIRE_UNARY(range_end > range_start);
    CHECK_EQ(ks[range_start].datetime, unified_start);
    CHECK_EQ(ks[range_end - 1].datetime, part.back().datetime);
    CHECK_EQ(range_end - range_start, part.size());
    /** @arg The reverse normalization is the exact inverse of the forward one */
    CHECK_EQ(sm.unifiedToMarketLocal(unified_start, "NASDAQ"), local_start);

    /** @arg Day lines are never converted: the label stays (no minutes, in-window, ordered) */
    auto days = ndx.getKRecordList(
      KQueryByDate(Datetime(201610240000LL), Datetime(201611110000LL), KQuery::DAY));
    REQUIRE_UNARY(days.size() >= 12);
    for (const auto& k : days) {
        REQUIRE_EQ(k.datetime.number() % 10000, (uint64_t)0);
        CHECK_UNARY(k.datetime >= Datetime(201610240000LL));
        CHECK_UNARY(k.datetime < Datetime(201611110000LL));
    }
    auto unordered_day = std::adjacent_find(
      days.begin(), days.end(),
      [](const KRecord& a, const KRecord& b) { return !(a.datetime < b.datetime); });
    CHECK_UNARY(unordered_day == days.end());
    /** @arg Day labels count the local trading days (none lost to the midnight crossing) */
    CHECK_UNARY(days.size() <= 14);
}

/** @} */
