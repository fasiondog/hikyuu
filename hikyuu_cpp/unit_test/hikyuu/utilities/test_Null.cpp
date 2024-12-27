/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

#include <doctest/doctest.h>
#include <cstdint>
#include <cmath>
#include <hikyuu/DataType.h>
#include <hikyuu/utilities/Null.h>

using namespace hku;

TEST_CASE("test_Null_size_t") {
    size_t null_size = Null<size_t>();
    CHECK_EQ(std::numeric_limits<std::size_t>::max(), null_size);
}

TEST_CASE("test_Null_double_float") {
    CHECK_UNARY(std::isnan(Null<double>()));
    CHECK_UNARY(std::isnan(Null<float>()));

    double null_double = Null<double>();
    double null_float = Null<float>();

    CHECK_EQ(Null<double>(), null_double);
    CHECK_EQ(null_double, Null<double>());
    CHECK_EQ(null_double, Null<float>());
    CHECK_EQ(null_float, Null<float>());
    CHECK_EQ(null_float, Null<double>());
    CHECK_UNARY(null_double == Null<double>());
    CHECK_UNARY(Null<double>() == null_double);
    CHECK_UNARY(null_double == Null<float>());
    CHECK_UNARY(Null<float>() == null_double);
    CHECK_UNARY(null_float == Null<float>());
    CHECK_UNARY(Null<float>() == null_float);
    CHECK_UNARY(null_float == Null<double>());
    CHECK_UNARY(Null<double>() == null_float);

    CHECK_NE(Null<double>(), 0.3);
    CHECK_NE(Null<float>(), 0.3);
    CHECK_NE(Null<double>(), 0.3f);
    CHECK_NE(Null<float>(), 0.3f);
    CHECK_NE(0.3, Null<double>());
    CHECK_NE(0.3, Null<float>());
    CHECK_NE(0.3f, Null<double>());
    CHECK_NE(0.3f, Null<float>());
}

TEST_CASE("test_Null_price_t") {
    CHECK_UNARY(std::isnan(Null<price_t>()));

    double null_price = Null<price_t>();

    CHECK_EQ(Null<price_t>(), null_price);
    CHECK_EQ(null_price, Null<price_t>());
    CHECK_UNARY(null_price == Null<price_t>());
    CHECK_UNARY(Null<price_t>() == null_price);

    CHECK_NE(Null<price_t>(), 0.3);
    CHECK_NE(Null<price_t>(), 0.3f);
    CHECK_NE(0.3, Null<price_t>());
    CHECK_NE(0.3f, Null<price_t>());
}