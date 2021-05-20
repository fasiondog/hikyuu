/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-21
 *     Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/db_connect/DBCondition.h>
#include <hikyuu/Log.h>

using namespace hku;

TEST_CASE("test_DBCondition") {
    DBCondition d1;
    HKU_INFO(d1.str());

    DBCondition d2("name", "test");
    HKU_INFO(d2.str());

    DBCondition d3("value", 3.14);
    HKU_INFO(d3.str());

    DBCondition d4("value", 3);
    HKU_INFO(d4.str());

    d2 = d2 & d4 | d3;
    HKU_INFO(d2.str());
}