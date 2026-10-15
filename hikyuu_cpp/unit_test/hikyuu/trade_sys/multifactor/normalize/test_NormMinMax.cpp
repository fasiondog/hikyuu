/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-15
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/DataType.h>
#include <hikyuu/trade_sys/multifactor/buildin_norm.h>

using namespace hku;

/** @par Test points */
TEST_CASE("test_NormMinMax") {
    NormPtr norm = NORM_MinMax();

    /** @arg the normal min-max maps the valid range to [0, 1] */
    PriceList ret = norm->normalize(PriceList{1.0, 2.0, 3.0, 4.0});
    REQUIRE_EQ(ret.size(), 4);
    CHECK(ret[0] == doctest::Approx(0.0));
    CHECK(ret[1] == doctest::Approx(1.0 / 3.0));
    CHECK(ret[2] == doctest::Approx(2.0 / 3.0));
    CHECK(ret[3] == doctest::Approx(1.0));

    /** @arg a nan at the front must not poison the whole cross-section */
    ret = norm->normalize(PriceList{Null<price_t>(), 1.0, 2.0, 3.0});
    REQUIRE_EQ(ret.size(), 4);
    CHECK_UNARY(std::isnan(ret[0]));
    CHECK(ret[1] == doctest::Approx(0.0));
    CHECK(ret[2] == doctest::Approx(0.5));
    CHECK(ret[3] == doctest::Approx(1.0));

    /** @arg the interior nan stays nan while the others map normally */
    ret = norm->normalize(PriceList{2.0, Null<price_t>(), 4.0});
    REQUIRE_EQ(ret.size(), 3);
    CHECK(ret[0] == doctest::Approx(0.0));
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK(ret[2] == doctest::Approx(1.0));

    /** @arg the all-nan input returns all nan, not a fake 0.0 */
    PriceList all_nan = {Null<price_t>(), Null<price_t>(), Null<price_t>()};
    ret = norm->normalize(all_nan);
    REQUIRE_EQ(ret.size(), 3);
    CHECK_UNARY(std::isnan(ret[0]));
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK_UNARY(std::isnan(ret[2]));

    /** @arg the empty input returns empty */
    ret = norm->normalize(PriceList());
    CHECK_UNARY(ret.empty());
}

/** @par Test points */
TEST_CASE("test_NormMinMax_zero_variance") {
    NormPtr norm = NORM_MinMax();

    /** @arg a zero-range cross-section returns the neutral 0 for the valid positions */
    PriceList ret = norm->normalize(PriceList{3.0, 3.0, 3.0, 3.0});
    REQUIRE_EQ(ret.size(), 4);
    for (size_t i = 0; i < ret.size(); i++) {
        CHECK(ret[i] == doctest::Approx(0.0));
    }

    /** @arg the nan positions stay nan while the equal valid positions become 0 */
    ret = norm->normalize(PriceList{5.0, Null<price_t>(), 5.0});
    REQUIRE_EQ(ret.size(), 3);
    CHECK(ret[0] == doctest::Approx(0.0));
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK(ret[2] == doctest::Approx(0.0));
}
