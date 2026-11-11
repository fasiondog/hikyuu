/*
 * test_RateTable.cpp
 *
 *  Created on: 2026-11-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/RateTable.h>
#include <hikyuu/utilities/exception.h>

using namespace hku;

/**
 * @defgroup test_RateTable test_RateTable
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points: the value domain, the atomic commit and the empty table */
TEST_CASE("test_RateTable_parse") {
    RateTable table;

    /** @arg a fresh table is empty */
    CHECK_UNARY(table.empty());

    /** @arg an empty expression gives an empty table */
    CHECK_NOTHROW(table.parse("", "RT", false, false, true));
    CHECK_UNARY(table.empty());

    /** @arg a rate above one is rejected by the default domain but accepted for an exchange rate */
    CHECK_THROWS_AS(table.parse("NASDAQ:7.15", "RT", false, false), hku::exception);
    CHECK_NOTHROW(table.parse("NASDAQ:7.15", "RT", false, false, true));
    CHECK_UNARY(!table.empty());

    /** @arg zero is rejected unless allow_zero is set, and a negative rate is always rejected */
    CHECK_THROWS_AS(table.parse("NASDAQ:0", "RT", false, false, true), hku::exception);
    CHECK_THROWS_AS(table.parse("NASDAQ:-1", "RT", true, false, true), hku::exception);
    CHECK_NOTHROW(table.parse("HK:0", "RT", true, false, true));
    CHECK_NOTHROW(table.parse("SH:0.5", "RT", false, false, true));

    /** @arg one is rejected unless allow_one is set */
    CHECK_THROWS_AS(table.parse("SZ:1.0", "RT", false, false), hku::exception);
    CHECK_NOTHROW(table.parse("SZ:1.0", "RT", false, true));

    /** @arg a malformed item is rejected */
    CHECK_THROWS_AS(table.parse("NASDAQ", "RT", false, false, true), hku::exception);
    CHECK_THROWS_AS(table.parse("NASDAQ:abc", "RT", false, false, true), hku::exception);
    CHECK_THROWS_AS(table.parse("NASDAQ:7x", "RT", false, false, true), hku::exception);
    CHECK_THROWS_AS(table.parse(":7.0", "RT", false, false, true), hku::exception);

    /** @arg a rejected expression leaves the previously committed table intact */
    Stock us = StockManager::instance().getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());
    CHECK_NOTHROW(table.parse("NASDAQ:7.0, HK:0.9", "RT", false, false, true));
    CHECK_THROWS_AS(table.parse("NASDAQ:0, SH:0.8", "RT", false, false, true), hku::exception);
    CHECK_EQ(table.lookup(us, 1.0), 7.0);
    CHECK_UNARY(!table.has(StockManager::instance().getStock("sh600004")));

    /** @arg parsing an empty expression clears the table */
    CHECK_NOTHROW(table.parse("", "RT", false, false, true));
    CHECK_UNARY(table.empty());
    CHECK_EQ(table.lookup(us, 1.0), 1.0);
}

/** @par Test points: the type key wins over the market key, the market code is case insensitive,
 * and an unmatched security falls back */
TEST_CASE("test_RateTable_lookup") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!sh.isNull());
    REQUIRE_UNARY(!us.isNull());

    const string type_key = std::to_string(sh.type());

    /** @arg the security type takes precedence over its market */
    RateTable table;
    table.parse("sh:0.5, " + type_key + ":0.25", "RT", false, false, true);
    CHECK_EQ(table.lookup(sh, 1.0), 0.25);
    CHECK_UNARY(table.has(sh));

    /** @arg an unmatched security falls back and is not reported as present */
    CHECK_EQ(table.lookup(us, 1.0), 1.0);
    CHECK_UNARY(!table.has(us));

    /** @arg the smallest ratio of the whole table is returned */
    CHECK_EQ(table.minRate(1.0), 0.25);

    /** @arg a market key matches regardless of its case */
    RateTable by_market;
    by_market.parse("nasdaq:7.15", "RT", false, false, true);
    CHECK_EQ(by_market.lookup(us, 1.0), 7.15);
    CHECK_UNARY(by_market.has(us));
    CHECK_UNARY(!by_market.has(sh));
    CHECK_EQ(by_market.minRate(1.0), 1.0);

    /** @arg the fallback of an empty table is returned for every lookup */
    RateTable empty;
    CHECK_EQ(empty.lookup(sh, 3.0), 3.0);
    CHECK_UNARY(!empty.has(sh));
    CHECK_EQ(empty.minRate(3.0), 3.0);
}

/** @} */
