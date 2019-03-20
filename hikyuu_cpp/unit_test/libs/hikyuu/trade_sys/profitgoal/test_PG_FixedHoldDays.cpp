/*
 * test_ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_sys_suite
    #include <boost/test/unit_test.hpp>
#endif

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
BOOST_AUTO_TEST_CASE( test_PG_FixedHoldDays) {
    StockManager& sm = StockManager::instance();
    TMPtr tm = crtTM(Datetime(199001010000LL), 100000);

    Datetime start_date(199911100000LL); //测试起始日期
    Datetime end_date(200002250000LL);   //测试结束日期
    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    Stock stk = sm.getStock("sh600000");
    KData k = stk.getKData(query);

    PGPtr pg = PG_FixedHoldDays();
    pg->setTM(tm);
    pg->setTO(k);

    tm->buy(Datetime(199911110000LL), stk, 29.51, 100, 0.0, 0.0, 0.0, PART_SIGNAL);

    price_t null_price = Null<price_t>();

    /** @arg 检查默认参数 */
    BOOST_CHECK(pg->getParam<int>("days") == 5);

    /** @arg days = 0 */
    pg->setParam<int>("days", 0);
    BOOST_CHECK(pg->getGoal(Datetime(199911100000LL), 0.0) == 0.0);
    BOOST_CHECK(pg->getGoal(Datetime(199911110000LL), 0.0) == 0.0);
    BOOST_CHECK(pg->getGoal(Datetime(199911120000LL), 0.0) == 0.0);

    /** @arg days = 1 */
    pg->setParam<int>("days", 1);
    BOOST_CHECK(pg->getGoal(Datetime(199911100000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911110000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911120000LL), 0.0) == 0.0);
    BOOST_CHECK(pg->getGoal(Datetime(199911130000LL), 0.0) == 0.0);
    BOOST_CHECK(pg->getGoal(Datetime(199911150000LL), 0.0) == 0.0);

    /** @arg days = 2 */
    pg->setParam<int>("days", 2);
    BOOST_CHECK(pg->getGoal(Datetime(199911100000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911110000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911120000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911130000LL), 0.0) == 0.0);
    BOOST_CHECK(pg->getGoal(Datetime(199911150000LL), 0.0) == 0.0);
    BOOST_CHECK(pg->getGoal(Datetime(199911160000LL), 0.0) == 0.0);

    /** @arg days = 3 */
    pg->setParam<int>("days", 3);
    BOOST_CHECK(pg->getGoal(Datetime(199911100000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911110000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911120000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911130000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911150000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911160000LL), 0.0) == 0.0);

    /** @arg days = 4 */
    pg->setParam<int>("days", 4);
    BOOST_CHECK(pg->getGoal(Datetime(199911100000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911110000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911120000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911130000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911150000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911160000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911170000LL), 0.0) == 0.0);

    /** @arg days = 8 */
    pg->setParam<int>("days", 8);
    BOOST_CHECK(pg->getGoal(Datetime(199911100000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911110000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911120000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911130000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911150000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911160000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911170000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911180000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911190000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911200000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911210000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911220000LL), 0.0) == null_price);
    BOOST_CHECK(pg->getGoal(Datetime(199911230000LL), 0.0) == 0.0);
}

/** @} */



