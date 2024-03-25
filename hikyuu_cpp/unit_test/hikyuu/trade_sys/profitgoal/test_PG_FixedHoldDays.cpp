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

/** @par 检测点 */
TEST_CASE("test_PG_FixedHoldDays") {
    StockManager& sm = StockManager::instance();
    TMPtr tm = crtTM(Datetime(199001010000LL), 100000);

    Datetime start_date(199911100000LL);  // 测试起始日期
    Datetime end_date(200002250000LL);    // 测试结束日期
    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    Stock stk = sm.getStock("sh600000");
    KData k = stk.getKData(query);

    PGPtr pg = PG_FixedHoldDays();
    pg->setTM(tm);
    pg->setTO(k);

    tm->buy(Datetime(199911110000LL), stk, 29.51, 100, 0.0, 0.0, 0.0, PART_SIGNAL);

    /** @arg 检查默认参数 */
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

/** @} */
