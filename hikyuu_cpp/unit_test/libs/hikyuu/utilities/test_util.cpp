/*
 * test_util.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_utilities
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/Log.h>
#include <hikyuu/utilities/util.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_util test_hikyuu_util
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_round ) {
    double x;

    x = 10.11;
    BOOST_CHECK(roundEx(x) == 10.0);
    BOOST_CHECK(roundDown(x) == 10.0);
    BOOST_CHECK(roundUp(x) == 11.0);
    BOOST_CHECK(roundEx(x, 1) == 10.1);
    BOOST_CHECK(roundDown(x, 1) == 10.1);
    BOOST_CHECK(roundUp(x, 1) == 10.2);

    x = 10.55;
    BOOST_CHECK(roundEx(x) == 11);
    BOOST_CHECK(roundDown(x) == 10);
    BOOST_CHECK(roundUp(x) == 11.0);
    BOOST_CHECK(roundEx(x, 1) == 10.6);
    BOOST_CHECK(roundDown(x, 1) == 10.5);
    BOOST_CHECK(roundUp(x, 1) == 10.6);

    x = -10.11;
    BOOST_CHECK(roundEx(x) == -10);
    BOOST_CHECK(roundDown(x) == -10);
    BOOST_CHECK(roundUp(x) == -11.0);
    BOOST_CHECK(roundEx(x, 1) == -10.1);
    BOOST_CHECK(roundDown(x, 1) == -10.1);
    BOOST_CHECK(roundUp(x, 1) == -10.2);

    x = -10.55;
    BOOST_CHECK(roundEx(x) == -11);
    BOOST_CHECK(roundDown(x) == -10);
    BOOST_CHECK(roundUp(x) == -11.0);
    BOOST_CHECK(roundEx(x, 1) == -10.6);
    BOOST_CHECK(roundDown(x, 1) == -10.5);
    BOOST_CHECK(roundUp(x, 1) == -10.6);
}

/** @} */


