/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-09-29
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/DataType.h>
#include <hikyuu/trade_sys/multifactor/normalize/NormQuantile.h>
#include <hikyuu/trade_sys/multifactor/normalize/NormQuantileUniform.h>

using namespace hku;

/** @par Test points */
TEST_CASE("test_NormQuantile_nan_preserved") {
    NormQuantile norm;

    /** @arg the positions without value remain nan, not a fake 0.0 score (ISS-082) */
    PriceList src = {1.0, Null<price_t>(), 3.0, Null<price_t>(), 2.0};
    PriceList ret = norm.normalize(src);
    REQUIRE_EQ(ret.size(), src.size());
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK_UNARY(std::isnan(ret[3]));
    CHECK_UNARY(!std::isnan(ret[0]));
    CHECK_UNARY(!std::isnan(ret[2]));
    CHECK_UNARY(!std::isnan(ret[4]));

    /** @arg the valid values are ranked in ascending order (1.0 < 2.0 < 3.0) */
    CHECK_LT(ret[0], ret[4]);
    CHECK_LT(ret[4], ret[2]);

    /** @arg the all-nan input returns all nan */
    PriceList all_nan = {Null<price_t>(), Null<price_t>(), Null<price_t>()};
    PriceList ret2 = norm.normalize(all_nan);
    REQUIRE_EQ(ret2.size(), all_nan.size());
    CHECK_UNARY(std::isnan(ret2[0]));
    CHECK_UNARY(std::isnan(ret2[1]));
    CHECK_UNARY(std::isnan(ret2[2]));

    /** @arg the empty input returns empty */
    PriceList ret3 = norm.normalize(PriceList());
    CHECK_UNARY(ret3.empty());

    /** @arg the valid-only input produces no nan */
    PriceList ret4 = norm.normalize(PriceList{1.0, 2.0, 3.0, 4.0});
    REQUIRE_EQ(ret4.size(), 4);
    for (size_t i = 0; i < ret4.size(); ++i) {
        CHECK_UNARY(!std::isnan(ret4[i]));
    }
}

/** @par Test points */
TEST_CASE("test_NormQuantileUniform_nan_preserved") {
    NormQuantileUniform norm;

    /** @arg the positions without value remain nan, not a fake 0.0 score (ISS-082) */
    PriceList src = {1.0, Null<price_t>(), 3.0, Null<price_t>(), 2.0};
    PriceList ret = norm.normalize(src);
    REQUIRE_EQ(ret.size(), src.size());
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK_UNARY(std::isnan(ret[3]));
    CHECK_UNARY(!std::isnan(ret[0]));
    CHECK_UNARY(!std::isnan(ret[2]));
    CHECK_UNARY(!std::isnan(ret[4]));

    /** @arg the valid values are ranked in ascending order (1.0 < 2.0 < 3.0) */
    CHECK_LT(ret[0], ret[4]);
    CHECK_LT(ret[4], ret[2]);

    /** @arg the all-nan input returns all nan */
    PriceList all_nan = {Null<price_t>(), Null<price_t>(), Null<price_t>()};
    PriceList ret2 = norm.normalize(all_nan);
    REQUIRE_EQ(ret2.size(), all_nan.size());
    CHECK_UNARY(std::isnan(ret2[0]));
    CHECK_UNARY(std::isnan(ret2[1]));
    CHECK_UNARY(std::isnan(ret2[2]));

    /** @arg the empty input returns empty */
    PriceList ret3 = norm.normalize(PriceList());
    CHECK_UNARY(ret3.empty());

    /** @arg the valid-only input produces no nan */
    PriceList ret4 = norm.normalize(PriceList{1.0, 2.0, 3.0, 4.0});
    REQUIRE_EQ(ret4.size(), 4);
    for (size_t i = 0; i < ret4.size(); ++i) {
        CHECK_UNARY(!std::isnan(ret4[i]));
    }
}
