/*
 * test_MM_FixedUnits.cpp
 *
 *  Created on: 2026-10-18
 *  Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedUnits.h>

using namespace hku;

/**
 * @defgroup test_MM_FixedUnits test_MM_FixedUnits
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedUnits") {
    // sh000001: minTradeNumber == 1, unit == 1, so the numbers below are not clipped
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg n = 2, risk = 2: base = max(currentCash, initCash) / n = 5000, size = 2500 */
    auto tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    auto mm = MM_FixedUnits(2);
    mm->setTM(tm);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 2.0, 2.0, PART_SIGNAL), 2500.0);

    /** @arg the size is cash-constrained when the cash cannot afford the base:
     * base 5000 / risk 2 = 2500 shares needs 25000, only 10000 affords 1000 shares */
    tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    mm = MM_FixedUnits(2);
    mm->setTM(tm);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 2.0, PART_SIGNAL), 1000.0);

    /** @arg the cash below the initial capital falls back to the initial capital */
    tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    mm = MM_FixedUnits(2);
    mm->setTM(tm);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 2.0, 2.0, PART_SIGNAL), 2500.0);
    REQUIRE_UNARY(
      tm->buy(Datetime(200001010000), stock, 2.0, 2500, 0.0, 0.0, 2.0, PART_SIGNAL).business ==
      BUSINESS_BUY);  // cash 5000 < initCash 10000
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 2.0, 2.0, PART_SIGNAL), 2500.0);
}

/** @par Test points */
TEST_CASE("test_MM_FixedUnits_short") {
    // sh000001: minTradeNumber == 1, unit == 1
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg the short entry uses the same formula as the long buy: max(cash, initCash) / n /
     * risk with the short per-share risk (the stop-loss price minus the short entry price) */
    auto tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    auto mm = MM_FixedUnits(2);
    mm->setTM(tm);
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001010000), stock, 2.0, 2.0, PART_SIGNAL), 2500.0);

    /** @arg the short cover liquidates the entire position */
    CHECK_UNARY(
      (mm->getBuyShortNumber(Datetime(200001010000), stock, 2.0, 2.0, PART_SIGNAL) == MAX_DOUBLE));
}

/** @par Test points */
TEST_CASE("test_MM_FixedUnits_query_no_side_effect") {
    // The quantity query is read-only: repeated queries neither append trade records nor change
    // the cash
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    auto tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    auto mm = MM_FixedUnits(2);
    mm->setTM(tm);
    mm->getBuyNumber(Datetime(200001010000), stock, 2.0, 2.0, PART_SIGNAL);
    size_t record_count = tm->getTradeList().size();
    price_t cash = tm->currentCash();
    for (int i = 0; i < 5; ++i) {
        CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 2.0, 2.0, PART_SIGNAL), 2500.0);
        CHECK_EQ(tm->getTradeList().size(), record_count);
        CHECK_EQ(tm->currentCash(), cash);
    }
}

/** @} */
