/*
 * test_MM_FixedCapitalFunds.cpp
 *
 *  Created on: 2026-10-19
 *  Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCapitalFunds.h>

using namespace hku;

/**
 * @defgroup test_MM_FixedCapitalFunds test_MM_FixedCapitalFunds
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedCapitalFunds") {
    // sh000001: minTradeNumber == 1, unit == 1, so the numbers below are not clipped
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg the parameter bounds: capital > 0 */
    CHECK_THROWS_AS(MM_FixedCapitalFunds(0.0), std::exception);
    CHECK_THROWS_AS(MM_FixedCapitalFunds(-1.0), std::exception);
    REQUIRE_UNARY(MM_FixedCapitalFunds(100.0));

    Datetime dt(200001040000);  // a trading day, so the market value is available

    /** @arg the formula with no position: total assets == cash = 10000 -> 10000 / 100 = 100
     * shares; auto_checkin keeps the raw total-assets-based size (the cash top-up is done by
     * System at the actual trade) */
    auto tm = crtTM(Datetime(200001040000), 10000.0, TC_Zero());
    auto mm = MM_FixedCapitalFunds(100.0);
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", true);
    CHECK_EQ(mm->getBuyNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), 100.0);

    /** @arg the base is the total assets instead of the cash: after buying 100 shares the cash
     * drops to 9000 while the total assets keep the position market value, so the size grows
     * with the market value while a cash-based sizing would shrink to 90 */
    REQUIRE_UNARY(tm->buy(dt, stock, 10.0, 100, 0.0, 0.0, 10.0, PART_SIGNAL).business ==
                  BUSINESS_BUY);
    CHECK_EQ(tm->currentCash(), 9000.0);
    FundsRecord funds = tm->getFunds(dt, KQuery::DAY);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    CHECK_EQ(funds.market_value, 100 * close);
    double expected = int64_t(funds.net_assets() / 100.0);
    CHECK_NE(expected, 90.0);
    CHECK_EQ(mm->getBuyNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), expected);

    /** @arg the query is side-effect free: repeated calls keep the cash and the records */
    size_t records = tm->getTradeList().size();
    price_t cash = tm->currentCash();
    CHECK_EQ(mm->getBuyNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), expected);
    CHECK_EQ(tm->getTradeList().size(), records);
    CHECK_EQ(tm->currentCash(), cash);
}

/** @par Test points */
TEST_CASE("test_MM_FixedCapitalFunds_short") {
    // sh000001: minTradeNumber == 1, unit == 1
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg the short entry uses the same formula as the long buy: total assets / capital, and
     * the per-share risk is not used */
    Datetime dt(200001040000);
    auto tm = crtTM(Datetime(200001040000), 10000.0, TC_Zero());
    auto mm = MM_FixedCapitalFunds(100.0);
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", true);
    CHECK_EQ(mm->getSellShortNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), 100.0);
    CHECK_EQ(mm->getSellShortNumber(dt, stock, 10.0, 100.0, PART_SIGNAL), 100.0);

    /** @arg the short cover liquidates the entire position */
    CHECK_UNARY((mm->getBuyShortNumber(dt, stock, 10.0, 1.0, PART_SIGNAL) == MAX_DOUBLE));
}

/** @par Test points */
TEST_CASE("test_MM_FixedCapitalFunds_net_assets_base") {
    // sh000001: minTradeNumber == 1, unit == 1
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg in a margin account the borrowed stock value inflates the total assets but not the
     * net assets: the size follows the net assets (100000 / 100 = 1000 shares) */
    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    auto tm = crtTM(dt, 100000.0, TC_Zero());
    tm->setParam<bool>("support_borrow_stock", true);
    REQUIRE_UNARY(tm->sellShort(dt, stock, close, 100, 0.0, 0.0, close, PART_SIGNAL).business ==
                  BUSINESS_SELL_SHORT);
    FundsRecord funds = tm->getFunds(dt, KQuery::DAY);
    CHECK_EQ(funds.net_assets(), 100000.0);
    CHECK_UNARY(funds.total_assets() > funds.net_assets());

    auto mm = MM_FixedCapitalFunds(100.0);
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", true);  // keep the raw size, no cash constraint
    CHECK_EQ(mm->getBuyNumber(dt, stock, close, 1.0, PART_SIGNAL), 1000.0);
    CHECK_EQ(mm->getSellShortNumber(dt, stock, close, 1.0, PART_SIGNAL), 1000.0);
}

/** @} */
