/*
 * test_FixedPercent_SL.cpp
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
#include <hikyuu/trade_sys/stoploss/crt/ST_FixedPercent.h>

using namespace hku;

/**
 * @defgroup test_FixedPercent_SL test_FixedPercent_SL
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_FixedPercent_SL) {
    /** @arg p = 0.2 */
    StoplossPtr sp = ST_FixedPercent(0.2);
    price_t result = sp->getPrice(Datetime(200101010000), 10.0);
    BOOST_CHECK(std::fabs(result - 8.0) < 0.01);

    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sz000001");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    sp->setTO(kdata);
    result = sp->getPrice(Datetime(199101030000), 66.4);
    BOOST_CHECK(std::fabs(result - 53.12) < 0.01);

    /** @arg p = 0.0 */
    sp = ST_FixedPercent(0.0);
    result = sp->getPrice(Datetime(199101030000), 66.397);
    BOOST_CHECK(std::fabs(result - 66.4) < 0.01);

    /** @arg p = 1.0 */
    sp = ST_FixedPercent(1.0);
    result = sp->getPrice(Datetime(199101030000), 66.397);
    BOOST_CHECK(std::fabs(result - 0) < 0.01);
}

/** @} */


