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
#include <hikyuu/trade_manage/crt/TC_Zero.h>

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
     * lot multiple */
    TradeRecord tr = strategy.order(stock, 333);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, 300);

    /** @arg The buy request over maxTradeNumber is clamped to maxTradeNumber */
    tr = strategy.order(stock, stock.maxTradeNumber() * 2.0);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, stock.maxTradeNumber());

    /** @arg The sell request with an exact lot multiple only sells the requested quantity instead
     * of liquidating */
    tr = strategy.order(stock, -200);
    CHECK_EQ(tr.business, BUSINESS_SELL);
    CHECK_EQ(tr.number, 200);
    CHECK_EQ(tm->getHoldNumber(Datetime::now(), stock), stock.maxTradeNumber() + 100);

    /** @arg The sell request containing an odd lot on a clean lot-aligned position is truncated to
     * the lot multiple instead of liquidating */
    tr = strategy.order(stock, -150);
    CHECK_EQ(tr.business, BUSINESS_SELL);
    CHECK_EQ(tr.number, 100);
    CHECK_EQ(tm->getHoldNumber(Datetime::now(), stock), stock.maxTradeNumber());

    /** @arg num == -MAX_DOUBLE explicitly liquidates the whole position */
    tr = strategy.order(stock, 500);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, 500);
    tr = strategy.order(stock, -MAX_DOUBLE);
    CHECK_EQ(tr.business, BUSINESS_SELL);
    CHECK_EQ(tr.number, stock.maxTradeNumber() + 500);
    CHECK_EQ(tm->getHoldNumber(Datetime::now(), stock), 0);

    /** @arg A sell request below the min trade number is rejected */
    tr = strategy.order(stock, -50);
    CHECK_EQ(tr.business, BUSINESS_INVALID);

    /** @arg The sell request covering a position with an odd lot sells the whole position to carry
     * the odd lot away */
    REQUIRE_UNARY(tm->checkinStock(Datetime::now(), stock, 10.0, 150));
    tr = strategy.order(stock, -150);
    CHECK_EQ(tr.business, BUSINESS_SELL);
    CHECK_EQ(tr.number, 150);
    CHECK_EQ(tm->getHoldNumber(Datetime::now(), stock), 0);
}

/** @par Test points */
TEST_CASE("test_Strategy_orderValue_unit_cash_estimate") {
    // Both stocks quote at price 10.0 on today's daily bar; TC_Zero makes cost.total == 0 so the
    // expected numbers are exact. tickValue / tick == 2.0 makes stock.unit() != 1
    KRecord today_bar(Datetime::now());
    today_bar.openPrice = today_bar.highPrice = today_bar.lowPrice = today_bar.closePrice = 10.0;

    Stock unit1_stock("TEST", "OVU1", "OrderValue Unit1 Test", 1, true, Datetime(199001010000),
                      Datetime(209901010000), 0.01, 0.01, 2, 100, 1000000);
    unit1_stock.setKRecordList({today_bar}, KQuery::DAY);
    REQUIRE_UNARY(!unit1_stock.isNull());
    CHECK_EQ(unit1_stock.unit(), 1.0);

    Stock unit2_stock("TEST", "OVU2", "OrderValue Unit2 Test", 1, true, Datetime(199001010000),
                      Datetime(209901010000), 1.0, 2.0, 2, 100, 1000000);
    unit2_stock.setKRecordList({today_bar}, KQuery::DAY);
    REQUIRE_UNARY(!unit2_stock.isNull());
    CHECK_EQ(unit2_stock.unit(), 2.0);

    /** @arg unit == 1: the estimate keeps the original behavior, value 10000 buys 1000 shares */
    Strategy strategy1("test_Strategy_orderValue_unit1");
    auto tm1 = crtTM(Datetime(200001010000), 1000000.0, TC_Zero());
    strategy1.setTM(tm1);
    TradeRecord tr = strategy1.orderValue(unit1_stock, 10000.0);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, 1000);

    /** @arg unit == 1: the number is converted into a lot multiple before the cash decrement loop,
     *  value 9567 with cash 9200 buys 900 shares (the old fractional start under-bought 800) */
    Strategy strategy2("test_Strategy_orderValue_unit1_lotalign");
    auto tm2 = crtTM(Datetime(200001010000), 9200.0, TC_Zero());
    strategy2.setTM(tm2);
    tr = strategy2.orderValue(unit1_stock, 9567.0);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, 900);

    /** @arg unit == 2: the estimate multiplies by stock.unit(), value 10000 with cash 10000
     *  decrements to the affordable 500 shares (regression for the missing unit, ISS-135; the old
     *  code bought 1000 shares costing 20000) */
    Strategy strategy3("test_Strategy_orderValue_unit2");
    auto tm3 = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    strategy3.setTM(tm3);
    tr = strategy3.orderValue(unit2_stock, 10000.0);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, 500);

    /** @arg unit == 2: even one minimum trade quantity is unaffordable, an invalid record returns
     */
    Strategy strategy4("test_Strategy_orderValue_unit2_zero");
    auto tm4 = crtTM(Datetime(200001010000), 15.0, TC_Zero());
    strategy4.setTM(tm4);
    tr = strategy4.orderValue(unit2_stock, 1500.0);
    CHECK_EQ(tr.business, BUSINESS_INVALID);
}

/** @} */
