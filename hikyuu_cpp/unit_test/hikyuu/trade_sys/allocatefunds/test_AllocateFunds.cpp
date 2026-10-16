/*
 * test_AllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Unit tests for the portfolio-level fund allocation (AF) base parameters and the
 *  fixed-weight-list parameter validation.
 */

#include "doctest/doctest.h"
#include <limits>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_FixedWeightList.h>

using namespace hku;

/**
 * @defgroup test_AllocateFunds test_AllocateFunds
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_AllocateFunds_base_params") {
    AFPtr af = AF_EqualWeight();

    /** @arg the base parameter family and their defaults */
    CHECK_EQ(af->getParam<double>("max-single-position"), doctest::Approx(1.0));
    CHECK_EQ(af->getParam<double>("max-total-position"), doctest::Approx(1.0));
    CHECK_EQ(af->getParam<double>("fixed-amount"), doctest::Approx(0.0));
    CHECK_EQ(af->getParam<bool>("adjust_running_sys"), true);

    /** @arg the legacy base-class weight-list parameter has been removed (the fixed weights go
     * through AF_FixedWeightList) */
    CHECK_THROWS_AS(af->getParam<string>("weight-list"), std::exception);

    /** @arg the mode defaults to A */
    CHECK_EQ(af->getMode(), "A");
}

/** @par Test points */
TEST_CASE("test_AF_FixedWeightList_checkParam") {
    /** @arg a valid non-empty weight list is accepted */
    CHECK_NOTHROW(AF_FixedWeightList(vector<double>{1.0, 2.0, 3.0}));

    /** @arg an all-zero weight list is rejected (would actively clear the holdings otherwise) */
    CHECK_THROWS_AS(AF_FixedWeightList(vector<double>{0.0, 0.0, 0.0}), std::exception);

    /** @arg a negative weight is rejected */
    CHECK_THROWS_AS(AF_FixedWeightList(vector<double>{-1.0, 2.0}), std::exception);

    /** @arg a non-finite (inf) weight is rejected */
    CHECK_THROWS_AS(
      AF_FixedWeightList(vector<double>{std::numeric_limits<double>::infinity(), 1.0}),
      std::exception);

    /** @arg an overflowing sum (each item finite but the sum is inf) is rejected */
    CHECK_THROWS_AS(AF_FixedWeightList(vector<double>{1e308, 1e308}), std::exception);

    /** @arg an empty weight list returns the equal weight (does not construct a fixed-list AF) */
    CHECK_NOTHROW(AF_FixedWeightList(vector<double>{}));
}

/** @} */
