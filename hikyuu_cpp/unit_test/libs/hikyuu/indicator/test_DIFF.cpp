/*
 * test_DIFF.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/indicator/crt/DIFF.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_DIFF test_indicator_DIFF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_DIFF ) {
    /** @arg 正常测试 */
    PriceList d;
    for (size_t i = 0; i < 10; ++i) {
        d.push_back(i);
    }

    Indicator ind = PRICELIST(d);
    Indicator diff = DIFF(ind);
    BOOST_CHECK(diff.size() == 10);
    BOOST_CHECK(diff.discard() == 1);
    BOOST_CHECK(diff[0] == Null<price_t>());
    for (size_t i = 1; i < 10; ++i) {
        BOOST_CHECK(diff[i] == d[i] - d[i-1]);
    }

    /** @arg operator */
    diff = DIFF();
    Indicator expect = DIFF(ind);
    Indicator result = diff(ind);
    BOOST_CHECK(expect.size() == result.size());
    for (size_t i = 0; i < expect.size(); ++i) {
        BOOST_CHECK(result[i] == expect[i]);
    }
}

/** @} */


