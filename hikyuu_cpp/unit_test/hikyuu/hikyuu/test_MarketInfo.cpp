/*
 * test_MarketInfo.cpp
 *
 *  Created on: 2026-11-07
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/MarketInfo.h>
#include <hikyuu/utilities/Log.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_MarketInfo test_hikyuu_MarketInfo
 * @ingroup test_hikyuu_base_suite
 * @{
 */

namespace {
/* A market carrying only the timezone columns that matter here; sessions default to 09:30-16:00 */
MarketInfo makeMarket(int tz, int dst, TimeDelta o1 = Hours(9) + Minutes(30),
                      TimeDelta c1 = Hours(16), TimeDelta o2 = TimeDelta(),
                      TimeDelta c2 = TimeDelta()) {
    return MarketInfo("TST", "test market", "for the timezone conversion", "000001",
                      Datetime(202601010000LL), o1, c1, o2, c2, tz, dst, 0);
}
}  // namespace

/** @par Test points */
TEST_CASE("test_MarketInfo_localToUnified") {
    MarketInfo sh = makeMarket(480, 0);

    /** @arg Identity market: the local wall clock is already the unified expression */
    CHECK_EQ(sh.localToUnified(Datetime(202607011000LL)), Datetime(202607011000LL));
    CHECK_EQ(sh.unifiedToLocal(Datetime(202607011000LL)), Datetime(202607011000LL));

    /** @arg A null input is returned as is, on both directions */
    CHECK_UNARY(sh.localToUnified(Datetime()).isNull());
    CHECK_UNARY(sh.unifiedToLocal(Datetime()).isNull());

    /** @arg The default-constructed (null) market is identity as well, no shift is invented */
    MarketInfo null_market;
    CHECK_EQ(null_market.localToUnified(Datetime(202607011000LL)), Datetime(202607011000LL));

    MarketInfo us = makeMarket(-300, 1);

    /** @arg Daylight period: local 10:00 is Beijing 22:00 (+12h) */
    CHECK_EQ(us.localToUnified(Datetime(202607011000LL)), Datetime(202607012200LL));
    /** @arg Standard period: local 10:00 is Beijing 23:00 (+13h) */
    CHECK_EQ(us.localToUnified(Datetime(202601051000LL)), Datetime(202601052300LL));
    /** @arg The conversion crosses to the next Beijing day (local close 16:00 = 04:00 next day) */
    CHECK_EQ(us.localToUnified(Datetime(202607011600LL)), Datetime(202607020400LL));
    /** @arg The same crossing in the standard period: 16:00 = 05:00 next day */
    CHECK_EQ(us.localToUnified(Datetime(202601051600LL)), Datetime(202601060500LL));

    /** @arg unifiedToLocal is the exact inverse in both periods */
    CHECK_EQ(us.unifiedToLocal(Datetime(202607012200LL)), Datetime(202607011000LL));
    CHECK_EQ(us.unifiedToLocal(Datetime(202601052300LL)), Datetime(202601051000LL));
}

/** @par Test points */
TEST_CASE("test_MarketInfo_unifiedToLocal") {
    MarketInfo eu = makeMarket(0, 2);

    /** @arg EU rule switches at 01:00 UTC: local 10:00 inside daylight is Beijing 17:00 */
    CHECK_EQ(eu.localToUnified(Datetime(202607011000LL)), Datetime(202607011700LL));
    /** @arg Outside daylight the same wall clock is Beijing 18:00 */
    CHECK_EQ(eu.localToUnified(Datetime(202601051000LL)), Datetime(202601051800LL));
    /** @arg Inverse relation */
    CHECK_EQ(eu.unifiedToLocal(Datetime(202607011700LL)), Datetime(202607011000LL));

    /** @arg A zero offset without a rule (FX / crypto) is UTC: unified is 8h ahead of local */
    MarketInfo flat = makeMarket(0, 0);
    CHECK_EQ(flat.localToUnified(Datetime(202607011000LL)), Datetime(202607011800LL));
    CHECK_EQ(flat.unifiedToLocal(Datetime(202607011800LL)), Datetime(202607011000LL));
}

/** @par Test points */
TEST_CASE("test_MarketInfo_dstSwitchByYear") {
    MarketInfo us = makeMarket(-300, 1);

    /** @arg Before 2007 the old rule applies: the third Sunday of March is still standard (+13h) */
    CHECK_EQ(us.localToUnified(Datetime(200603190930LL)), Datetime(200603192230LL));
    /** @arg From 2007 the current rule applies: the same position is already daylight (+12h) */
    CHECK_EQ(us.localToUnified(Datetime(200703180930LL)), Datetime(200703182130LL));
    /** @arg On the autumn switching day, after 06:00 UTC the standard offset is back */
    CHECK_EQ(us.localToUnified(Datetime(202611011000LL)), Datetime(202611012300LL));
    /** @arg The repeated local hour [01:00,02:00) resolves to the standard side, always the same */
    CHECK_EQ(us.localToUnified(Datetime(202611010130LL)), Datetime(202611011430LL));

    /** @arg Both pre-images of the repeated hour map back to the same local clock (1h apart) */
    CHECK_EQ(us.unifiedToLocal(Datetime(202611011430LL)), Datetime(202611010130LL));
    CHECK_EQ(us.unifiedToLocal(Datetime(202611011330LL)), Datetime(202611010130LL));
}

/** @par Test points */
TEST_CASE("test_MarketInfo_roundTrip") {
    MarketInfo us = makeMarket(-300, 1);
    MarketInfo eu = makeMarket(0, 2);
    MarketInfo cn = makeMarket(480, 0);

    /** @arg Round trips over several years and the months holding a transition */
    for (long year : {2006L, 2007L, 2019L, 2026L}) {
        for (long month : {1L, 3L, 6L, 7L, 10L, 11L, 12L}) {
            for (long hhmm : {930L, 1000L, 1230L, 1600L}) {
                Datetime local(year, month, 15, hhmm / 100, hhmm % 100);
                for (const auto& market : {us, eu, cn}) {
                    CHECK_EQ(market.unifiedToLocal(market.localToUnified(local)), local);
                }
            }
        }
    }
}

/** @par Test points */
TEST_CASE("test_MarketInfo_hasAmbiguousSession") {
    /** @arg A session reaching the local [01:00,02:00) hour is flagged */
    CHECK_UNARY(makeMarket(0, 0, Hours(1), Hours(2) + Minutes(30)).hasAmbiguousSession());
    /** @arg A near full-day session is flagged */
    CHECK_UNARY(makeMarket(-300, 0, Hours(0), Hours(23) + Minutes(59)).hasAmbiguousSession());
    /** @arg Both session pairs equal means a continuous 24h market, which is flagged */
    CHECK_UNARY(makeMarket(-300, 0, TimeDelta(), TimeDelta()).hasAmbiguousSession());
    /** @arg Normal sessions do not touch the ambiguous hour */
    CHECK_UNARY_FALSE(makeMarket(-300, 0).hasAmbiguousSession());
    /** @arg A second session that is unset (0/0 alone) is not a 24h market */
    CHECK_UNARY_FALSE(makeMarket(-300, 1).hasAmbiguousSession());
    /** @arg Ending exactly at 01:00 or starting exactly at 02:00 do not overlap (half-open) */
    CHECK_UNARY_FALSE(makeMarket(0, 0, Hours(0), Hours(1)).hasAmbiguousSession());
    CHECK_UNARY_FALSE(makeMarket(0, 0, Hours(2), Hours(3)).hasAmbiguousSession());
    /** @arg A reversed session is a configuration error, so it never reaches the ambiguous hour */
    CHECK_UNARY_FALSE(makeMarket(0, 0, Hours(21), Hours(1) + Minutes(30)).hasAmbiguousSession());
}

/** @par Test points */
TEST_CASE("test_MarketInfo_construction_check") {
    /** @arg A daylight market reaching the ambiguous hour is rejected at construction */
    CHECK_THROWS_AS(makeMarket(-300, 1, Hours(1), Hours(2) + Minutes(30)), std::exception);
    /** @arg A continuous 24h market cannot carry a daylight rule either */
    CHECK_THROWS_AS(makeMarket(-300, 1, TimeDelta(), TimeDelta()), std::exception);
    /** @arg Without a daylight rule the check does not run */
    CHECK_NOTHROW(makeMarket(-300, 0, Hours(1), Hours(2) + Minutes(30)));
    /** @arg A normal US session with the US rule is accepted */
    CHECK_NOTHROW(makeMarket(-300, 1));
}

/** @} */
