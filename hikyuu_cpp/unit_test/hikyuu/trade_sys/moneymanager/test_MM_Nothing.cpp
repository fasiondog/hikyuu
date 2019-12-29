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

/** @par 检测点 */
TEST_CASE("test_MM_Nothing") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm;
    size_t result;

    /** @arg 初始无资金 */
    tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedA());
    MoneyManagerPtr mm = MM_Nothing();
    mm->setTM(tm);
    result = mm->getBuyNumber(Datetime(200101010000), stock, 10.0, 10.0, PART_SIGNAL);
    CHECK_EQ(result, 0);

    /** @arg 初始资金1292，零成本算法，不自动存入资金 */
    tm = crtTM(Datetime(199001010000LL), 1292, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 100);

    /** @arg 初始资金1292，固定成本算法，不自动存入资金 */
    tm = crtTM(Datetime(199001010000LL), 1292, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 0);

    /** @arg 初始资金1292*2，零成本算法，不自动存入资金 */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 200);

    /** @arg 初始资金1292*2，固定成本算法，不自动存入资金 */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 100);

    /** @arg 初始资金1292*2，固定成本算法，自动存入资金 */
    tm = crtTM(Datetime(199001010000LL), 1292 * 2, TC_FixedA());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", true);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 200);

    /** @arg 初始资金可买数超过最大可买数，零成本，不自动存入资金 */
    tm = crtTM(Datetime(199001010000LL), 12.92 * 2000000LL, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", false);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 1000000L);

    /** @arg 初始资金可买数超过最大可买数，零成本，自动存入资金 */
    tm = crtTM(Datetime(199001010000LL), 12.92 * 2000000LL, TC_Zero());
    mm = MM_Nothing();
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", true);
    result = mm->getBuyNumber(Datetime(200304160000), stock, 12.92, 12.92, PART_SIGNAL);
    CHECK_EQ(result, 1000000L);
}

/** @} */
