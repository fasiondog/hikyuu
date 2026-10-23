/*
 * test_AllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Unit tests for the portfolio-level fund allocation (AF) base parameters and the
 *  fixed-weight-list parameter validation.
 */

#include "doctest/doctest.h"
#include <cmath>
#include <limits>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_FixedAmount.h>
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
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(1.0));
    CHECK_EQ(af->getParam<double>("max_total_position"), doctest::Approx(1.0));
    CHECK_EQ(af->getParam<double>("fixed_amount"), doctest::Approx(0.0));
    CHECK_EQ(af->getParam<bool>("adjust_running_sys"), true);

    /** @arg the legacy base-class weight-list parameter has been removed (the fixed weights go
     * through AF_FixedWeightList) */
    CHECK_UNARY(!af->haveParam("weight-list"));

    /** @arg the deprecated hyphenated parameter names are rejected instead of registering
     * dead keys */
    CHECK_THROWS_AS(af->setParam<double>("max-single-position", 0.5), std::exception);
    CHECK_THROWS_AS(af->setParam<double>("max-total-position", 0.5), std::exception);
    CHECK_THROWS_AS(af->setParam<double>("fixed-amount", 0.5), std::exception);

    /** @arg non-default values are accepted and read back unchanged */
    af->setParam<double>("max_single_position", 0.5);
    af->setParam<double>("max_total_position", 0.8);
    af->setParam<double>("fixed_amount", 30000.0);
    af->setParam<bool>("adjust_running_sys", false);
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(0.5));
    CHECK_EQ(af->getParam<double>("max_total_position"), doctest::Approx(0.8));
    CHECK_EQ(af->getParam<double>("fixed_amount"), doctest::Approx(30000.0));
    CHECK_EQ(af->getParam<bool>("adjust_running_sys"), false);

    /** @arg max_single_position is not bounded from below (<=0 means no limit, no clamp on set) */
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 0.0));
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(0.0));
    CHECK_NOTHROW(af->setParam<double>("max_single_position", -1.0));
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(-1.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_checkParam") {
    AFPtr af = AF_EqualWeight();

    /** @arg a NaN max_single_position is rejected (it would silently disable the concentration cap)
     */
    CHECK_THROWS_AS(af->setParam<double>("max_single_position", std::nan("")), std::exception);

    /** @arg a NaN max_total_position is rejected (it would silently disable the total budget cap)
     */
    CHECK_THROWS_AS(af->setParam<double>("max_total_position", std::nan("")), std::exception);

    /** @arg non-positive / >1 values stay valid: <=0 means no limit and >1 is clamped in use */
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 0.0));
    CHECK_NOTHROW(af->setParam<double>("max_single_position", -1.0));
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 2.0));
    CHECK_NOTHROW(af->setParam<double>("max_total_position", 0.0));
    CHECK_NOTHROW(af->setParam<double>("max_total_position", 2.0));

    /** @arg fixed_amount: 0 (disabled) and a positive amount are valid */
    CHECK_NOTHROW(af->setParam<double>("fixed_amount", 0.0));
    CHECK_NOTHROW(af->setParam<double>("fixed_amount", 30000.0));

    /** @arg fixed_amount: a negative value is rejected (it would silently fall back to weights) */
    CHECK_THROWS_AS(af->setParam<double>("fixed_amount", -1.0), std::exception);

    /** @arg fixed_amount: NaN / inf are rejected */
    CHECK_THROWS_AS(af->setParam<double>("fixed_amount", std::nan("")), std::exception);
    CHECK_THROWS_AS(af->setParam<double>("fixed_amount", std::numeric_limits<double>::infinity()),
                    std::exception);

    /** @arg the AF_FixedAmount factory goes through the same validation */
    CHECK_NOTHROW(AF_FixedAmount(20000.0));
    CHECK_THROWS_AS(AF_FixedAmount(-1.0), std::exception);
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
    CHECK_EQ(AF_FixedWeightList(PriceList{})->name(), "AF_EqualWeight");
}

/** @} */
