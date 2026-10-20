/*
 * test_ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/profitgoal/crt/PG_FixedHoldDays.h>

using namespace hku;

/**
 * @defgroup test_ProfitGoal test_ProfitGoal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_PG_FixedHoldDays") {
    StockManager& sm = StockManager::instance();
    TMPtr tm = crtTM(Datetime(199001010000LL), 100000);

    Datetime start_date(199911100000LL);  // Test start date
    Datetime end_date(200002250000LL);    // Test end date
    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    Stock stk = sm.getStock("sh600000");
    KData k = stk.getKData(query);

    PGPtr pg = PG_FixedHoldDays();
    pg->setTM(tm);
    pg->setTO(k);

    tm->buy(Datetime(199911110000LL), stk, 29.51, 100, 0.0, 0.0, 0.0, PART_SIGNAL);

    /** @arg Check the default parameters */
    CHECK_EQ(pg->getParam<int>("days"), 5);

    /** @arg days = 0 */
    CHECK_THROWS_AS(pg->setParam<int>("days", 0), std::exception);

    /** @arg days = 1 */
    pg->setParam<int>("days", 1);
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911100000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911110000LL), 0.0)));
    CHECK_EQ(pg->getGoal(Datetime(199911120000LL), 0.0), 0.0);
    CHECK_EQ(pg->getGoal(Datetime(199911130000LL), 0.0), 0.0);
    CHECK_EQ(pg->getGoal(Datetime(199911150000LL), 0.0), 0.0);

    /** @arg days = 2 */
    pg->setParam<int>("days", 2);
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911100000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911110000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911120000LL), 0.0)));
    CHECK_EQ(pg->getGoal(Datetime(199911130000LL), 0.0), 0.0);
    CHECK_EQ(pg->getGoal(Datetime(199911150000LL), 0.0), 0.0);
    CHECK_EQ(pg->getGoal(Datetime(199911160000LL), 0.0), 0.0);

    /** @arg days = 3 */
    pg->setParam<int>("days", 3);
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911100000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911110000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911120000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911130000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911150000LL), 0.0)));
    CHECK_EQ(pg->getGoal(Datetime(199911160000LL), 0.0), 0.0);

    /** @arg days = 4 */
    pg->setParam<int>("days", 4);
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911100000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911110000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911120000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911130000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911150000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911160000LL), 0.0)));
    CHECK_EQ(pg->getGoal(Datetime(199911170000LL), 0.0), 0.0);

    /** @arg days = 8 */
    pg->setParam<int>("days", 8);
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911100000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911110000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911120000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911130000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911150000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911160000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911170000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911180000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911190000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911200000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911210000LL), 0.0)));
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911220000LL), 0.0)));
    CHECK_EQ(pg->getGoal(Datetime(199911230000LL), 0.0), 0.0);
}

/** @arg the short side forces a close by returning the current price once the hold days are
 *  reached; before that or with no short position there is no goal */
TEST_CASE("test_PG_FixedHoldDays_short") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh600000");
    KData k = stk.getKData(
      KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY));

    TMPtr tm = crtTM(Datetime(199001010000LL), 100000);
    tm->setParam<bool>("support_borrow_stock", true);
    tm->sellShort(Datetime(199911110000LL), stk, 29.51, 100);

    PGPtr pg = PG_FixedHoldDays();
    pg->setTM(tm);
    pg->setTO(k);
    pg->setParam<int>("days", 1);

    /** @arg the take day itself is not counted, no goal yet */
    CHECK_UNARY(std::isnan(pg->getShortGoal(Datetime(199911110000LL), 30.0)));

    /** @arg one trading day later the close is forced: the goal equals the current price so that
     *  the system's price <= goal always holds */
    CHECK_EQ(pg->getShortGoal(Datetime(199911120000LL), 30.0), 30.0);
    CHECK_EQ(pg->getShortGoal(Datetime(199911130000LL), 25.5), 25.5);

    /** @arg with no short position there is no goal */
    TMPtr tm_empty = crtTM(Datetime(199001010000LL), 100000);
    PGPtr pg2 = PG_FixedHoldDays();
    pg2->setTM(tm_empty);
    pg2->setTO(k);
    pg2->setParam<int>("days", 1);
    CHECK_UNARY(std::isnan(pg2->getShortGoal(Datetime(199911120000LL), 30.0)));

    /** @arg the tm released by reset yields no goal */
    pg->reset();
    CHECK_UNARY(std::isnan(pg->getShortGoal(Datetime(199911120000LL), 30.0)));
}

/** @par Test points */
TEST_CASE("test_PG_FixedHoldDays_clone") {
    StockManager& sm = StockManager::instance();
    TMPtr tm = crtTM(Datetime(199001010000LL), 100000);

    Datetime start_date(199911100000LL);  // Test start date
    Datetime end_date(200002250000LL);    // Test end date
    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    Stock stk = sm.getStock("sh600000");
    KData k = stk.getKData(query);

    tm->buy(Datetime(199911110000LL), stk, 29.51, 100, 0.0, 0.0, 0.0, PART_SIGNAL);

    PGPtr pg = PG_FixedHoldDays();
    pg->setTM(tm);
    pg->setTO(k);
    pg->setParam<int>("days", 1);

    PGPtr cloned = pg->clone();

    /** @arg the clone does not carry the TO of the original */
    CHECK_UNARY(cloned->getTO().empty());

    /** @arg before setTO there is no trading-day list, so the hold-days rule never triggers */
    CHECK_UNARY(std::isnan(cloned->getGoal(Datetime(199911120000LL), 30.0)));

    /** @arg setTO on the clone re-runs the calculation, the goal is available */
    cloned->setTO(k);
    CHECK_EQ(cloned->getGoal(Datetime(199911120000LL), 0.0), 0.0);

    /** @arg the tm is released by reset, no goal is set */
    pg->reset();
    CHECK_UNARY(std::isnan(pg->getGoal(Datetime(199911120000LL), 0.0)));
}

/** @} */
