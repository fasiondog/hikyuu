/*
 * test_ZONGGUBEN.cpp
 *
 *  Created on: 2026-10-13
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ZONGGUBEN.h>

using namespace hku;

/**
 * @defgroup test_indicator_ZONGGUBEN test_indicator_ZONGGUBEN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points: index has no weight data -> discard equals size */
TEST_CASE("test_ZONGGUBEN_index_no_data") {
    KData k = getKData("sh000001", KQueryByIndex(-100));
    REQUIRE(k.size() > 0);

    Indicator zgb = ZONGGUBEN(k);
    CHECK_EQ(zgb.name(), "ZONGGUBEN");
    CHECK_EQ(zgb.size(), k.size());
    /** @arg Empty StockWeight list -> m_discard = total (whole buffer Null) */
    CHECK_EQ(zgb.discard(), k.size());
}

/** @par Test points: leading bars before the first valid weight record stay Null */
TEST_CASE("test_ZONGGUBEN_leading_bars_before_first_record") {
    Stock stk = getStock("SH600004");
    REQUIRE(!stk.isNull());
    // K-line starts at 20030101; the first weight record is 20030428
    KData k = stk.getKData(KQueryByDate(Datetime(200301010000), Datetime(200708250000)));
    REQUIRE(k.size() > 10);

    Indicator zgb = ZONGGUBEN(k);
    CHECK_EQ(zgb.size(), k.size());

    Datetime first_record_date(200304280000);
    size_t i = 0;
    /** @arg Bars before the first valid record's date are Null */
    for (; i < zgb.size(); i++) {
        if (k[i].datetime < first_record_date) {
            CHECK_UNARY(std::isnan(zgb[i]));
        } else {
            break;
        }
    }
    /** @arg m_discard advances past the leading Null prefix via updateDiscard(true) */
    CHECK_EQ(zgb.discard(), i);

    /** @arg Values follow the totalCount sequence from the first record onward */
    for (; i < zgb.size(); i++) {
        if (k[i].datetime < Datetime(200712140000)) {
            CHECK_EQ(zgb[i], 100000.0);
        } else if (k[i].datetime < Datetime(200712270000)) {
            CHECK_EQ(zgb[i], 113500.0);
        } else {
            CHECK_EQ(zgb[i], 115000.0);
        }
    }
}

/** @par Test points: the incremental path matches full calculation */
TEST_CASE("test_ZONGGUBEN_incremental_matches_full") {
    Stock stk = getStock("SH600004");
    REQUIRE(!stk.isNull());
    REQUIRE(Indicator::enableIncrementCalculate());

    // Extend the back of the same context: front matches -> framework takes the increment path
    KData k_short = stk.getKData(KQueryByDate(Datetime(200301010000), Datetime(200708250000)));
    KData k_long = stk.getKData(KQueryByDate(Datetime(200301010000), Datetime(201108170000)));
    REQUIRE(k_short.size() > 0);
    REQUIRE(k_long.size() > k_short.size());

    Indicator zgb = ZONGGUBEN();
    zgb.setContext(k_short);
    zgb.setContext(k_long);  // triggers _increment_calculate with start_pos == k_short.size()

    Indicator full = ZONGGUBEN(k_long);
    /** @arg size, discard and every value match the single-shot full calculation */
    CHECK_EQ(zgb.size(), full.size());
    CHECK_EQ(zgb.discard(), full.discard());
    for (size_t i = 0; i < zgb.size(); i++) {
        if (std::isnan(full[i])) {
            CHECK_UNARY(std::isnan(zgb[i]));
        } else {
            CHECK_UNARY(!std::isnan(zgb[i]));
            CHECK_EQ(zgb[i], full[i]);
        }
    }
}

/** @} */
