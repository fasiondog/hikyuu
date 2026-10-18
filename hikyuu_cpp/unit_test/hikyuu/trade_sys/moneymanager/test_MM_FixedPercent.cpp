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

/** @} */
