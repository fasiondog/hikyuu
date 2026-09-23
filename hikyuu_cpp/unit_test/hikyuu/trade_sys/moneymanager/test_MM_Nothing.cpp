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
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 100);

    /** @arg The initial capital 1292 with a fixed cost algorithm, no automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 0);

    /** @arg The initial capital 1292*2 with a zero cost algorithm, no automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 200);

    /** @arg The initial capital 1292*2 with a fixed cost algorithm, no automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 100);

    /** @arg The initial capital 1292*2 with a fixed cost algorithm, an automatic deposit */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", true);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 200);

    /** @arg The buyable count of the capital exceeds the maximum, zero cost, no automatic deposit
     */
    tm = crtTM(Datetime(199001010000LL), 12.92 * 2000000LL, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 1000000L);

    /** @arg The buyable count of the capital exceeds the maximum, zero cost, an automatic deposit
     */
    tm = crtTM(Datetime(199001010000LL), 12.92 * 2000000LL, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", true);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 1000000L);
}

/** @} */
