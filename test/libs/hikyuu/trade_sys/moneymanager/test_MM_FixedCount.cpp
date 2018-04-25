/*
 * test_MM_FixedCount.cpp
 *
 *  Created on: 2013-4-19
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
#include <hikyuu/trade_manage/crt/TC_FixedA.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>

using namespace hku;

/**
 * @defgroup test_MM_FixedCount test_MM_FixedCount
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_MM_FixedCount ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedA());

    /** @arg n < 1 */
    MoneyManagerPtr mm = MM_FixedCount(0);
    mm->setTM(tm);
    int result = mm->getBuyNumber(Datetime(200101010000), stock, 10.0, 10.0, PART_SIGNAL);
    BOOST_CHECK(result == 0);

    /** @arg n = 100, 一个初始资金为0的交易账户，能够执行买入操作 */
    tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedA());
    BOOST_CHECK(tm->initCash() == 0.0);
    mm = MM_FixedCount(100);
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", true);
    mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, PART_SIGNAL);
    BOOST_CHECK(tm->cash(Datetime(200001200000)) == 2417.02);
}

/** @} */


