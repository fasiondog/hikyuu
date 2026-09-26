/*
 * test_Strategy.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-09-27
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/strategy/Strategy.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>

using namespace hku;

/**
 * @defgroup test_Strategy test_Strategy
 * @ingroup test_hikyuu_strategy_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Strategy_order") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    REQUIRE_UNARY(!stock.isNull());

    Strategy strategy("test_Strategy_order");
    auto tm = crtTM(Datetime(199001010000LL), 1000000.0, TC_FixedA());
    strategy.setTM(tm);

    /** @arg The buy request with a non-integer multiple of minTradeNumber is rounded down to the
     * lot multiple (ISS-120) */
    TradeRecord tr = strategy.order(stock, 333);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, 300);

    /** @arg The buy request over maxTradeNumber is clamped to maxTradeNumber */
    tr = strategy.order(stock, stock.maxTradeNumber() * 2.0);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, stock.maxTradeNumber());

    /** @arg The sell request with an exact lot multiple only sells the requested quantity instead
     * of liquidating (ISS-006) */
    tr = strategy.order(stock, -200);
    CHECK_EQ(tr.business, BUSINESS_SELL);
    CHECK_EQ(tr.number, 200);
    CHECK_EQ(tm->getHoldNumber(Datetime::now(), stock), stock.maxTradeNumber() + 100);

    /** @arg The sell request containing an odd lot (a non-integer multiple of minTradeNumber)
     * sells all the remaining position to carry the odd lot away (ISS-006) */
    tr = strategy.order(stock, -150);
    CHECK_EQ(tr.business, BUSINESS_SELL);
    CHECK_EQ(tr.number, stock.maxTradeNumber() + 100);
    CHECK_EQ(tm->getHoldNumber(Datetime::now(), stock), 0);

    /** @arg num == -MAX_DOUBLE explicitly liquidates the whole position */
    tr = strategy.order(stock, 500);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, 500);
    tr = strategy.order(stock, -MAX_DOUBLE);
    CHECK_EQ(tr.business, BUSINESS_SELL);
    CHECK_EQ(tr.number, 500);
    CHECK_EQ(tm->getHoldNumber(Datetime::now(), stock), 0);
}

/** @} */
