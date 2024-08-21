/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-21
 *     Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/db_connect/DBCondition.h>
#include "hikyuu/utilities/Log.h"
#include <fmt/ranges.h>

using namespace hku;

TEST_CASE("test_DBCondition") {
    DBCondition d = Field("name") == "test";
    CHECK_EQ(d.str(), R"((name="test"))");

    d = Field("name") != "test";
    CHECK_EQ(d.str(), R"((name<>"test"))");

    d = Field("name") >= "test";
    CHECK_EQ(d.str(), R"((name>="test"))");

    d = Field("name") <= "test";
    CHECK_EQ(d.str(), R"((name<="test"))");

    d = Field("name") > "test";
    CHECK_EQ(d.str(), R"((name>"test"))");

    d = Field("name") < "test";
    CHECK_EQ(d.str(), R"((name<"test"))");

    CHECK_EQ(Field("name").like("*test").str(), R"((name like "*test"))");

    d = Field("name") == std::string("test");
    CHECK_EQ(d.str(), R"((name="test"))");

    d = Field("name") != std::string("test");
    CHECK_EQ(d.str(), R"((name<>"test"))");

    d = Field("name") >= std::string("test");
    CHECK_EQ(d.str(), R"((name>="test"))");

    d = Field("name") <= std::string("test");
    CHECK_EQ(d.str(), R"((name<="test"))");

    d = Field("name") > std::string("test");
    CHECK_EQ(d.str(), R"((name>"test"))");

    d = Field("name") < std::string("test");
    CHECK_EQ(d.str(), R"((name<"test"))");

    CHECK_EQ(Field("name").like("*test").str(), R"((name like "*test"))");

    d = Field("name").in(std::vector<std::string>({"1", "2", "3"}));
    CHECK_EQ(d.str(), R"((name in ("1","2","3")))");

    d = Field("name").not_in(std::vector<std::string>({"1", "2", "3"}));
    CHECK_EQ(d.str(), R"((name not in ("1","2","3")))");

    CHECK_THROWS_AS(Field("name").in(std::vector<std::string>()), hku::exception);

    d = Field("id") == 1;
    CHECK_EQ(d.str(), "(id=1)");

    d = Field("id") != 1;
    CHECK_EQ(d.str(), "(id<>1)");

    d = Field("id") <= 1;
    CHECK_EQ(d.str(), "(id<=1)");

    d = Field("id") >= 1;
    CHECK_EQ(d.str(), "(id>=1)");

    d = Field("id") > 1;
    CHECK_EQ(d.str(), "(id>1)");

    d = Field("id") < 1;
    CHECK_EQ(d.str(), "(id<1)");

    CHECK_EQ(Field("id").in(std::vector<int>({1, 2, 3})).str(), "(id in (1,2,3))");
    CHECK_THROWS_AS(Field("id").in(std::vector<int>()), hku::exception);

    CHECK_EQ(Field("id").not_in(std::vector<int>({1, 2, 3})).str(), "(id not in (1,2,3))");
    CHECK_THROWS_AS(Field("id").not_in(std::vector<int>()), hku::exception);

    d = Field("id") == 3.14;
    CHECK_EQ(d.str(), "(id=3.14)");

    d = Field("id") != 3.14;
    CHECK_EQ(d.str(), "(id<>3.14)");

    d = Field("id") >= 3.14;
    CHECK_EQ(d.str(), "(id>=3.14)");

    d = Field("id") <= 3.14;
    CHECK_EQ(d.str(), "(id<=3.14)");

    d = Field("id") > 3.14;
    CHECK_EQ(d.str(), "(id>3.14)");

    d = Field("id") < 3.14;
    CHECK_EQ(d.str(), "(id<3.14)");

    CHECK_EQ(Field("id").in(std::vector<double>({3.14, 3.14, 3.14})).str(),
             "(id in (3.14,3.14,3.14))");

    CHECK_EQ(Field("id").not_in(std::vector<double>({3.14, 3.14, 3.14})).str(),
             "(id not in (3.14,3.14,3.14))");

    CHECK_THROWS_AS(Field("id").in(std::vector<double>()), hku::exception);
    CHECK_THROWS_AS(Field("id").not_in(std::vector<double>()), hku::exception);
}