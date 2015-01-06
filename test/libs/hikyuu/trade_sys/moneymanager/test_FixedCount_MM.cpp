/*
 * test_FixedCount_MM.cpp
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
#include <hikyuu/trade_manage/crt/crtFixedATC.h>
#include <hikyuu/trade_sys/moneymanager/crt/FixedCount_MM.h>

using namespace hku;

/**
 * @defgroup test_FixedCount_MM
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_FixedCount_MM ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    /** @arg n < 1 */
    MoneyManagerPtr mm = FixedCount_MM(0);
    int result = mm->getBuyNumber(Datetime(200101010000), stock, 10.0, 0.0);
    BOOST_CHECK(result == 0);

    /** @arg n = 100, 一个初始资金为0的交易账户，能够执行买入操作 */
    TradeManagerPtr tm = crtTM(Datetime(199001010000LL), 0.0, crtFixedATC());
    BOOST_CHECK(tm->initCash() == 0.0);
    mm = FixedCount_MM(100);
    mm->setTM(tm);
    mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 0.0);
    BOOST_CHECK(tm->cash(Datetime(200001200000)) == 2417.01);
}

/** @} */


