/*
 * test_MM_FixedCapital.cpp
 *
 *  Created on: 2026-10-19
 *  Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCapital.h>

using namespace hku;

/**
 * @defgroup test_MM_FixedCapital test_MM_FixedCapital
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedCapital") {
    // sh000001: minTradeNumber == 1, unit == 1, so the numbers below are not clipped
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg the parameter bounds: capital > 0 */
    CHECK_THROWS_AS(MM_FixedCapital(0.0), std::exception);
    CHECK_THROWS_AS(MM_FixedCapital(-1.0), std::exception);
    REQUIRE_UNARY(MM_FixedCapital(100.0));

    /** @arg the formula: cash / capital = 10000 / 100 = 100 shares */
    auto tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    auto mm = MM_FixedCapital(100.0);
    mm->setTM(tm);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 100.0);

    /** @arg the size follows the remaining cash: after buying 100 shares at 10 the cash is 9000
     * -> 90 shares */
    REQUIRE_UNARY(
      tm->buy(Datetime(200001010000), stock, 10.0, 100, 0.0, 0.0, 10.0, PART_SIGNAL).business ==
      BUSINESS_BUY);
    CHECK_EQ(tm->currentCash(), 9000.0);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 90.0);

    /** @arg the risk is not used by this strategy */
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 100.0, PART_SIGNAL), 90.0);

    /** @arg the query is side-effect free: repeated calls keep the cash and the records */
    size_t records = tm->getTradeList().size();
    price_t cash = tm->currentCash();
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 90.0);
    CHECK_EQ(tm->getTradeList().size(), records);
    CHECK_EQ(tm->currentCash(), cash);
}

/** @par Test points */
TEST_CASE("test_MM_FixedCapital_short") {
    // sh000001: minTradeNumber == 1, unit == 1
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg the short entry uses the same formula as the long buy: cash / capital, and the
     * per-share risk is not used */
    auto tm = crtTM(Datetime(200001010000), 10000.0, TC_Zero());
    auto mm = MM_FixedCapital(100.0);
    mm->setTM(tm);
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 100.0);
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001010000), stock, 10.0, 100.0, PART_SIGNAL),
             100.0);

    /** @arg the short cover liquidates the entire position */
    CHECK_UNARY(
      (mm->getBuyShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL) == MAX_DOUBLE));
}

/** @} */
