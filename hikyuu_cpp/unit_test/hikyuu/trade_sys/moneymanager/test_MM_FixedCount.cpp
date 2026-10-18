/*
 * test_MM_FixedCount.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>
#include <hikyuu/trade_manage/crt/TC_FixedAPre2015.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>

using namespace hku;

/**
 * @defgroup test_MM_FixedCount test_MM_FixedCount
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedCount") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedAPre2015());

    /** @arg n < 1 */
    CHECK_THROWS_AS(MM_FixedCount(0), std::exception);

    /** @arg n = 100; with auto_checkin the query deposits nothing (the top-up is done by System
     * at the actual trade) */
    tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedAPre2015());
    CHECK_EQ(tm->initCash(), 0.0);
    auto mm = MM_FixedCount(100);
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", true);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, PART_SIGNAL), 100.0);
    CHECK_EQ(tm->cash(Datetime(200001200000)), 0.0);
}

/** @} */
