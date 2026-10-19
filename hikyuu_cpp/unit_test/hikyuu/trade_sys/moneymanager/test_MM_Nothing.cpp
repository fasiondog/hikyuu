/*
 * test_MM_FixedCount.cpp
 *
 *  Created on: 2019-2-24
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>

using namespace hku;

/**
 * @defgroup test_MM_Nothing test_MM_Nothing
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_Nothing") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm;
    size_t result;

    /** @arg There is no initial capital */
    tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedA());
    MoneyManagerPtr mm = MM_Nothing();
    mm->setTM(tm);
    result = mm->getBuyNumber(Datetime(200101010000), stock, 10.0, 10.0, PART_SIGNAL);
    CHECK_EQ(result, 0);

    /** @arg The initial capital 1292 with a zero cost algorithm, no automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 100);

    /** @arg The initial capital 1292 with a fixed cost algorithm, no automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 0);

    /** @arg The initial capital 1292*2 with a zero cost algorithm, no automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 200);

    /** @arg The initial capital 1292*2 with a fixed cost algorithm, no automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 100);

    /** @arg The initial capital 1292*2 with a fixed cost algorithm, an automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", true);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 200);

    /** @arg The buyable count of the capital exceeds the maximum, zero cost, no automatic deposit
     */
    tm = crtTM(Datetime(199001010000LL), 12.92 * 2000000LL, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 1000000L);

    /** @arg The buyable count of the capital exceeds the maximum, zero cost, an automatic deposit
     */
    tm = crtTM(Datetime(199001010000LL), 12.92 * 2000000LL, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", true);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 1000000L);
}

/** @par Test points */
TEST_CASE("test_MM_Nothing_historical_cash") {
    // The cash base is the cash of the query datetime, not the latest one: a historical query
    // must not see the cash of the future trades
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    auto tm = crtTM(Datetime(199001010000LL), 1292.0, TC_Zero());
    auto mm = MM_Nothing();
    mm->setTM(tm);

    /** @arg the query before any trade returns the size from the initial cash (1292 / 10
     * aligned to the lot size) */
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 100.0);

    /** @arg a buy at a later datetime does not reduce the cash seen by the earlier query */
    CHECK_EQ(
      tm->buy(Datetime(200101010000), stock, 10.0, 100, 0.0, 0.0, 10.0, PART_SIGNAL).business,
      BUSINESS_BUY);
    CHECK_EQ(tm->currentCash(), 292.0);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 100.0);

    /** @arg the query after the buy sees the reduced cash (292 / 10 is below the lot size) */
    CHECK_EQ(mm->getBuyNumber(Datetime(200201010000), stock, 10.0, 1.0, PART_SIGNAL), 0.0);
}

/** @par Test points */
TEST_CASE("test_MM_Nothing_if_have_short_will_not_sell_short") {
    // sh000001: minTradeNumber == 1, unit == 1
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    auto tm = crtTM(dt, 100000.0, TC_Zero());
    tm->setParam<bool>("support_borrow_stock", true);
    auto mm = MM_Nothing();
    mm->setTM(tm);

    /** @arg by default an open short does not stop a further short entry: the size follows the
     * net assets (100000 / 10 = 10000) */
    CHECK_EQ(mm->getSellShortNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), 10000.0);

    /** @arg with the guard enabled a short entry is rejected once a short is open */
    mm->setParam<bool>("if_have_short_will_not_sell_short", true);
    CHECK_EQ(mm->getSellShortNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), 10000.0);
    REQUIRE_EQ(
      tm->sellShort(dt, stock, close, 1, 0.0, 0.0, close, PART_SIGNAL).business,
      BUSINESS_SELL_SHORT);
    CHECK_EQ(mm->getSellShortNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), 0.0);

    /** @arg a long position alone does not trigger the short guard */
    auto tm2 = crtTM(dt, 100000.0, TC_Zero());
    auto mm2 = MM_Nothing();
    mm2->setTM(tm2);
    mm2->setParam<bool>("if_have_short_will_not_sell_short", true);
    REQUIRE_EQ(tm2->buy(dt, stock, 10.0, 100, 0.0, 0.0, 10.0, PART_SIGNAL).business, BUSINESS_BUY);
    CHECK_UNARY(mm2->getSellShortNumber(dt, stock, 10.0, 1.0, PART_SIGNAL) > 0.0);
}

/** @} */
