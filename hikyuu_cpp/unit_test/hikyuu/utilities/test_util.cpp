/*
 * test_util.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/Log.h>
#include <hikyuu/utilities/util.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_util test_hikyuu_util
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_round") {
    double x;

    x = 10.11;
    CHECK(roundEx(x) == 10.0);
    CHECK(roundDown(x) == 10.0);
    CHECK(roundUp(x) == 11.0);
    CHECK(roundEx(x, 1) == 10.1);
    CHECK(roundDown(x, 1) == 10.1);
    CHECK(roundUp(x, 1) == 10.2);

    x = 10.55;
    CHECK(roundEx(x) == 11);
    CHECK(roundDown(x) == 10);
    CHECK(roundUp(x) == 11.0);
    CHECK(roundEx(x, 1) == 10.6);
    CHECK(roundDown(x, 1) == 10.5);
    CHECK(roundUp(x, 1) == 10.6);

    x = -10.11;
    CHECK(roundEx(x) == -10);
    CHECK(roundDown(x) == -10);
    CHECK(roundUp(x) == -11.0);
    CHECK(roundEx(x, 1) == -10.1);
    CHECK(roundDown(x, 1) == -10.1);
    CHECK(roundUp(x, 1) == -10.2);

    x = -10.55;
    CHECK(roundEx(x) == -11);
    CHECK(roundDown(x) == -10);
    CHECK(roundUp(x) == -11.0);
    CHECK(roundEx(x, 1) == -10.6);
    CHECK(roundDown(x, 1) == -10.5);
    CHECK(roundUp(x, 1) == -10.6);
}

/** @} */
