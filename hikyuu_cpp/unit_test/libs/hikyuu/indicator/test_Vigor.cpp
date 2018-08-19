/*
 * test_Vigor.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/VIGOR.h>

using namespace hku;

/**
 * @defgroup test_indicator_VIGOR test_indicator_VIGOR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_VIGOR ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata;
    Indicator vigor;

    /** @arg kdata为空时 */
    vigor = VIGOR(kdata, 1);
    BOOST_CHECK(vigor.size() == 0);
    BOOST_CHECK(vigor.empty() == true);

    /** @arg 正常情况 */
    KQuery query = KQuery(0, 10);
    kdata = stock.getKData(query);
    BOOST_CHECK(kdata.size() == 10);
    vigor = VIGOR(kdata, 1);
    BOOST_CHECK(vigor.discard() == 1);
    BOOST_CHECK(vigor.size()== 10);
    BOOST_CHECK(vigor.empty() == false);
    BOOST_CHECK(vigor[0] == Null<price_t>());
    BOOST_CHECK(std::fabs(vigor[1] + 11761.36) < 0.0001 );

    vigor = VIGOR(kdata, 2);
    BOOST_CHECK(vigor.discard() == 1);
}

/** @} */


