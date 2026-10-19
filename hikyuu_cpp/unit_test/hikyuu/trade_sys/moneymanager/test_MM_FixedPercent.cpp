/*
 * test_MM_FixedPercent.cpp
 *
 *  Created on: 2026-10-19
 *  Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedPercent.h>

using namespace hku;

/**
 * @defgroup test_MM_FixedPercent test_MM_FixedPercent
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedPercent") {
    // sh000001: minTradeNumber == 1, unit == 1, so the numbers below are not clipped
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg the parameter bounds: 0 < p <= 1.0 */
    CHECK_THROWS_AS(MM_FixedPercent(0.0), std::exception);
    CHECK_THROWS_AS(MM_FixedPercent(-0.1), std::exception);
    CHECK_THROWS_AS(MM_FixedPercent(1.1), std::exception);
    REQUIRE_UNARY(MM_FixedPercent(1.0));

    /** @arg the formula: cash * p / risk = 10000 * 0.03 / 1 = 300 shares */
    auto tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    auto mm = MM_FixedPercent(0.03);
    mm->setTM(tm);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 300.0);

    /** @arg the size is cash-constrained: 10000 * 0.03 / 0.1 = 3000 shares needs 30000, only
     * 10000 affords exactly 1000 shares */
    tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    mm = MM_FixedPercent(0.03);
    mm->setTM(tm);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 0.1, PART_SIGNAL), 1000.0);

    /** @arg the query is side-effect free: repeated calls keep the cash and the records */
    size_t records = tm->getTradeList().size();
    price_t cash = tm->currentCash();
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 0.1, PART_SIGNAL), 1000.0);
    CHECK_EQ(tm->getTradeList().size(), records);
    CHECK_EQ(tm->currentCash(), cash);
}

/** @par Test points */
TEST_CASE("test_MM_FixedPercent_short") {
    // sh000001: minTradeNumber == 1, unit == 1
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg the short entry uses the same formula as the long buy: cash * p / risk with the
     * short per-share risk (the stop-loss price minus the short entry price) */
    auto tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    auto mm = MM_FixedPercent(0.03);
    mm->setTM(tm);
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 300.0);

    /** @arg the short cover liquidates the entire position */
    CHECK_UNARY(
      (mm->getBuyShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL) == MAX_DOUBLE));
}

/** @par Test points */
TEST_CASE("test_MM_FixedPercent_short_equity_base") {
    // sh000001: minTradeNumber == 1, unit == 1
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg an existing short inflates the cash by the borrowed proceeds but not the net assets:
     * the short size follows the net assets (100000 * 0.03 / 10 = 300 shares), not the cash */
    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    auto tm = crtTM(dt, 100000.0, TC_Zero());
    tm->setParam<bool>("support_borrow_stock", true);
    REQUIRE_UNARY(tm->sellShort(dt, stock, close, 100, 0.0, 0.0, close, PART_SIGNAL).business ==
                  BUSINESS_SELL_SHORT);
    CHECK_EQ(tm->currentCash(), roundEx(100000.0 + 100 * close, tm->precision()));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), 100000.0);

    auto mm = MM_FixedPercent(0.03);
    mm->setTM(tm);
    CHECK_EQ(mm->getSellShortNumber(dt, stock, close, 10.0, PART_SIGNAL), 300.0);
}

/** @} */
