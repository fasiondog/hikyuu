/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-21
 *     Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/db_connect/DBCondition.h>
#include <hikyuu/Log.h>
#include <fmt/ranges.h>

using namespace hku;

TEST_CASE("test_DBCondition") {
    DBCondition d1;

    d1 = (Field("name") == "test" & Field("id") == 1 | Field("val") == 3.14f) + ASC("name");
    HKU_INFO(d1.str());

    HKU_INFO(Field("name") == 1);
    HKU_INFO(Field("name") == "test");
    HKU_INFO(Field("id").in<std::string>({"1", "2", "3"}));
}