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
TEST_CASE("test_NormZScore") {
    /** @arg the default non-recursive z-score is centered at 0 */
    NormPtr norm = NORM_Zscore();
    PriceList ret = norm->normalize(PriceList{1.0, 2.0, 3.0, 4.0, 5.0});
    REQUIRE_EQ(ret.size(), 5);
    price_t sum = 0.0;
    for (size_t i = 0; i < ret.size(); i++) {
        sum += ret[i];
    }
    CHECK(sum == doctest::Approx(0.0));

    /** @arg a nan position stays nan and the others are still symmetric around 0 */
    ret = norm->normalize(PriceList{1.0, Null<price_t>(), 3.0});
    REQUIRE_EQ(ret.size(), 3);
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK_UNARY(!std::isnan(ret[0]));
    CHECK_UNARY(!std::isnan(ret[2]));
    CHECK(ret[0] == doctest::Approx(-ret[2]));

    /** @arg out_extreme clips the outliers into [-nsigma, nsigma] (non-recursive) */
    norm = NORM_Zscore(true, 1.0, false);
    ret = norm->normalize(PriceList{1.0, 2.0, 3.0, 4.0, 100.0});
    REQUIRE_EQ(ret.size(), 5);
    for (size_t i = 0; i < ret.size(); i++) {
        CHECK_UNARY(!std::isnan(ret[i]));
        CHECK_UNARY(ret[i] <= 1.0);
        CHECK_UNARY(ret[i] >= -1.0);
    }

    /** @arg recursive de-extreme must terminate instead of overflowing the stack */
    norm = NORM_Zscore(true, 1.0, true);
    ret = norm->normalize(PriceList{1.0, 2.0, 3.0, 4.0, 100.0});
    REQUIRE_EQ(ret.size(), 5);
    for (size_t i = 0; i < ret.size(); i++) {
        CHECK_UNARY(!std::isnan(ret[i]));
        CHECK_UNARY(ret[i] <= 1.0 + 1e-9);
        CHECK_UNARY(ret[i] >= -1.0 - 1e-9);
    }

    /** @arg a degenerate single-element cross-section returns the neutral 0, not the raw value */
    ret = norm->normalize(PriceList{7.0});
    REQUIRE_EQ(ret.size(), 1);
    CHECK(ret[0] == doctest::Approx(0.0));
}

/** @par Test points */
TEST_CASE("test_NormZScore_zero_variance") {
    NormPtr norm = NORM_Zscore();

    /** @arg a zero-variance cross-section returns 0 for the valid positions instead of 0/0 nan */
    PriceList ret = norm->normalize(PriceList{2.0, 2.0, 2.0, 2.0});
    REQUIRE_EQ(ret.size(), 4);
    for (size_t i = 0; i < ret.size(); i++) {
        CHECK_UNARY(!std::isnan(ret[i]));
        CHECK(ret[i] == doctest::Approx(0.0));
    }

    /** @arg a zero-variance cross-section keeps nan as nan and the valid ones 0 */
    ret = norm->normalize(PriceList{7.0, Null<price_t>(), 7.0});
    REQUIRE_EQ(ret.size(), 3);
    CHECK(ret[0] == doctest::Approx(0.0));
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK(ret[2] == doctest::Approx(0.0));

    /** @arg a cross-section with a single valid point must not leak its raw magnitude: 0 */
    ret = norm->normalize(PriceList{5.0, Null<price_t>(), Null<price_t>()});
    REQUIRE_EQ(ret.size(), 3);
    CHECK(ret[0] == doctest::Approx(0.0));
    CHECK_UNARY(std::isnan(ret[1]));
    CHECK_UNARY(std::isnan(ret[2]));
}
