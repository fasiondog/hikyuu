/*
 * test_PG_FixedPercent.cpp
 *
 *  Created on: 2026-10-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/profitgoal/crt/PG_FixedPercent.h>

using namespace hku;

/**
 * @defgroup test_PG_FixedPercent test_PG_FixedPercent
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_PG_FixedPercent") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh600000");
    KData k =
      stk.getKData(KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY));

    TMPtr tm = crtTM(Datetime(199001010000LL), 100000);
    tm->buy(Datetime(199911110000LL), stk, 10.0, 100, 0.0, 0.0, 0.0, PART_SIGNAL);
    tm->buy(Datetime(199911120000LL), stk, 20.0, 100, 0.0, 0.0, 0.0, PART_SIGNAL);
    tm->sell(Datetime(199911150000LL), stk, 30.0, 100);

    PGPtr pg = PG_FixedPercent(0.2);
    pg->setTM(tm);
    pg->setTO(k);

    /** @arg the cost price is buyMoney / totalNumber, a partial sell keeps the target price
     *  unchanged (cost = 3000 / 200 = 15, goal = 18.0 without being pushed to the next tick
     *  by the float noise of 15 * 1.2) */
    CHECK_EQ(pg->getGoal(Datetime(199911160000LL), 0.0), 18.0);

    /** @arg the target price is rounded up to the stock precision (15 * 1.0014 = 15.021) */
    PGPtr pg_up = PG_FixedPercent(0.0014);
    pg_up->setTM(tm);
    pg_up->setTO(k);
    CHECK_EQ(pg_up->getGoal(Datetime(199911160000LL), 0.0), 15.03);

    /** @arg a target exactly on the tick grid is not pushed to the next tick by the float
     *  noise: no position yet, the cost is the current price (4.05 * 1.2 = 4.86, 1.85 * 1.2 = 2.22)
     */
    CHECK_EQ(pg->getGoal(Datetime(199911100000LL), 4.05), 4.86);
    CHECK_EQ(pg->getGoal(Datetime(199911100000LL), 1.85), 2.22);

    /** @arg no short position, the target price is based on the current price (5.75 * 0.8 = 4.6) */
    CHECK_EQ(pg->getShortGoal(Datetime(199911160000LL), 10.0), 8.0);
    CHECK_EQ(pg->getShortGoal(Datetime(199911160000LL), 5.75), 4.6);

    /** @arg the tm is null, no goal is set instead of a null pointer dereference */
    PGPtr pg_no_tm = PG_FixedPercent(0.2);
    pg_no_tm->setTO(k);
    CHECK_UNARY(std::isnan(pg_no_tm->getGoal(Datetime(199911160000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg_no_tm->getShortGoal(Datetime(199911160000LL), 10.0)));

    /** @arg the tm is released by reset, no goal is set */
    pg->reset();
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911160000LL), 0.0)));
}

/** @par Test points */
TEST_CASE("test_PG_FixedPercent_short_goal") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh600000");
    KData k =
      stk.getKData(KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY));

    TMPtr tm = crtTM(Datetime(199001010000LL), 100000);
    tm->setParam<bool>("support_borrow_stock", true);
    tm->sellShort(Datetime(199911110000LL), stk, 20.0, 100);
    tm->sellShort(Datetime(199911120000LL), stk, 10.0, 100);
    tm->buyShort(Datetime(199911150000LL), stk, 12.0, 100);

    PGPtr pg = PG_FixedPercent(0.2);
    pg->setTM(tm);
    pg->setTO(k);

    /** @arg the short cost price is sellMoney / totalNumber, a partial cover keeps the target
     *  price unchanged (cost = 3000 / 200 = 15, goal = 15 * 0.8 = 12; the sold quantity is only
     *  100, so a cost price based on it would give 24) */
    CHECK_EQ(pg->getShortGoal(Datetime(199911160000LL), 0.0), 12.0);

    /** @arg the short target price is rounded down to the stock precision
     *  (15 * 0.9986 = 14.979) */
    PGPtr pg_down = PG_FixedPercent(0.0014);
    pg_down->setTM(tm);
    pg_down->setTO(k);
    CHECK_EQ(pg_down->getShortGoal(Datetime(199911160000LL), 0.0), 14.97);
}

/** @arg the clone drops the TO, so before setTO it holds no position and falls back to using the
 *  current price as the cost basis */
TEST_CASE("test_PG_FixedPercent_clone_no_to") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh600000");
    KData k =
      stk.getKData(KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY));

    TMPtr tm = crtTM(Datetime(199001010000LL), 100000);
    tm->buy(Datetime(199911110000LL), stk, 10.0, 100, 0.0, 0.0, 0.0, PART_SIGNAL);

    PGPtr pg = PG_FixedPercent(0.2);
    pg->setTM(tm);
    pg->setTO(k);

    PGPtr cloned = pg->clone();
    CHECK_UNARY(cloned->getTO().empty());
    // no TO -> null stock -> no position -> cost = current price -> goal = 10.0 * 1.2 = 12.0
    CHECK_LT(std::fabs(cloned->getGoal(Datetime(199911160000LL), 10.0) - 12.0), 0.00001);
}

/** @} */
