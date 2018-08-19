/*
 * test_REF.cpp
 *
 *  Created on: 2015年3月21日
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/REF.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_REF test_indicator_REF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_REF ) {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg n = 0 */
    result = REF(data, 0);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i <10; ++i) {
        BOOST_CHECK(result[i] == data[i]);
    }

    /** @arg n = 1 */
    result = REF(data, 1);
    BOOST_CHECK(result.discard() == 1);
    for (int i = 1; i < 10; ++i) {
        BOOST_CHECK(result[i] == data[i-1]);
    }

    /** @arg n = 9 */
    result = REF(data, 9);
    BOOST_CHECK(result.discard() == 9);
    BOOST_CHECK(result[9] == data[0]);

    /** @arg n = 10 */
    result = REF(data, 10);
    BOOST_CHECK(result.discard() == 10);
}

/** @} */


