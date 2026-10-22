/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-09-29
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/DataType.h>
#include <hikyuu/trade_sys/multifactor/buildin_norm.h>

using namespace hku;

/** @par Test points */
TEST_CASE("test_NormQuantile_nan_preserved") {
    NormPtr norm = NORM_Quantile();

    /** @arg the positions without value remain nan, not a fake 0.0 score (ISS-082) */
    PriceList src = {1.0, Null<price_t>(), 3.0, Null<price_t>(), 2.0};
    PriceList ret = norm->normalize(src);
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
    PriceList ret2 = norm->normalize(all_nan);
    REQUIRE_EQ(ret2.size(), all_nan.size());
    CHECK_UNARY(std::isnan(ret2[0]));
    CHECK_UNARY(std::isnan(ret2[1]));
    CHECK_UNARY(std::isnan(ret2[2]));

    /** @arg the empty input returns empty */
    PriceList ret3 = norm->normalize(PriceList());
    CHECK_UNARY(ret3.empty());

    /** @arg the valid-only input produces no nan */
    PriceList ret4 = norm->normalize(PriceList{1.0, 2.0, 3.0, 4.0});
    REQUIRE_EQ(ret4.size(), 4);
    for (size_t i = 0; i < ret4.size(); ++i) {
        CHECK_UNARY(!std::isnan(ret4[i]));
    }
}

/** @par Test points */
TEST_CASE("test_NormQuantile_zero_dispersion") {
    /** @arg a zero-dispersion cross-section returns the neutral 0 for the valid positions */
    NormPtr norm = NORM_Quantile();
    PriceList ret = norm->normalize(PriceList{4.0, 4.0, 4.0, 4.0});
    REQUIRE_EQ(ret.size(), 4);
    for (size_t i = 0; i < ret.size(); ++i) {
        CHECK_UNARY(!std::isnan(ret[i]));
        CHECK(ret[i] == doctest::Approx(0.0));
    }

    /** @arg nan positions stay nan while the equal valid positions become 0 */
    ret = norm->normalize(PriceList{4.0, Null<price_t>(), 4.0});
    REQUIRE_EQ(ret.size(), 3);
    CHECK(ret[0] == doctest::Approx(0.0));
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK(ret[2] == doctest::Approx(0.0));

    /** @arg the uniform variant returns the midpoint 0.5 (its (0,1) neutral), not the lowest 0 */
    NormPtr un = NORM_Quantile_Uniform();
    ret = un->normalize(PriceList{9.0, 9.0, 9.0});
    REQUIRE_EQ(ret.size(), 3);
    for (size_t i = 0; i < ret.size(); ++i) {
        CHECK(ret[i] == doctest::Approx(0.5));
    }
}

/** @par Test points */
TEST_CASE("test_NormQuantile_quantile_boundary") {
    /** @arg (0, 1) is accepted and means no truncation: all ranks stay strictly increasing */
    PriceList src = {1.0, 2.0, 3.0, 4.0, 100.0};
    PriceList ret = NORM_Quantile(0.0, 1.0)->normalize(src);
    REQUIRE_EQ(ret.size(), src.size());
    for (size_t i = 1; i < ret.size(); ++i) {
        CHECK_LT(ret[i - 1], ret[i]);
    }

    /** @arg a bounded range clamps the upper tail into ties, different from the no-truncation
     * result */
    PriceList clamped = NORM_Quantile(0.0, 0.5)->normalize(src);
    REQUIRE_EQ(clamped.size(), src.size());
    CHECK(clamped[2] == doctest::Approx(clamped[4]));
    CHECK(clamped[3] == doctest::Approx(clamped[4]));

    /** @arg the uniform variant accepts the same open boundaries, all ranks stay strictly
     * increasing */
    PriceList uni = NORM_Quantile_Uniform(0.0, 1.0)->normalize(src);
    REQUIRE_EQ(uni.size(), src.size());
    for (size_t i = 1; i < uni.size(); ++i) {
        CHECK_LT(uni[i - 1], uni[i]);
    }

    /** @arg out-of-range boundaries are rejected */
    CHECK_THROWS_AS(NORM_Quantile(-0.1, 1.0), std::exception);
    CHECK_THROWS_AS(NORM_Quantile(0.0, 1.1), std::exception);
    CHECK_THROWS_AS(NORM_Quantile_Uniform(-0.1, 1.0), std::exception);
    CHECK_THROWS_AS(NORM_Quantile_Uniform(0.0, 1.1), std::exception);

    /** @arg an inverted (equal) boundary is still rejected */
    CHECK_THROWS_AS(NORM_Quantile(0.5, 0.5), std::exception);
    CHECK_THROWS_AS(NORM_Quantile_Uniform(0.5, 0.5), std::exception);
}

/** @par Test points */
TEST_CASE("test_NormQuantileUniform_nan_preserved") {
    NormPtr norm = NORM_Quantile_Uniform();

    /** @arg the positions without value remain nan, not a fake 0.0 score (ISS-082) */
    PriceList src = {1.0, Null<price_t>(), 3.0, Null<price_t>(), 2.0};
    PriceList ret = norm->normalize(src);
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
    PriceList ret2 = norm->normalize(all_nan);
    REQUIRE_EQ(ret2.size(), all_nan.size());
    CHECK_UNARY(std::isnan(ret2[0]));
    CHECK_UNARY(std::isnan(ret2[1]));
    CHECK_UNARY(std::isnan(ret2[2]));

    /** @arg the empty input returns empty */
    PriceList ret3 = norm->normalize(PriceList());
    CHECK_UNARY(ret3.empty());

    /** @arg the valid-only input produces no nan */
    PriceList ret4 = norm->normalize(PriceList{1.0, 2.0, 3.0, 4.0});
    REQUIRE_EQ(ret4.size(), 4);
    for (size_t i = 0; i < ret4.size(); ++i) {
        CHECK_UNARY(!std::isnan(ret4[i]));
    }
}
