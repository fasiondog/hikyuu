/*
 * test_AllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Unit tests for the portfolio-level fund allocation (AF) weight-list parsing and the
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
TEST_CASE("test_AllocateFunds_parseWeightList") {
    AFPtr af = AF_EqualWeight();

    /** @arg an empty weight-list returns an empty vector (the caller falls back to the equal
     * weight) */
    af->setParam<string>("weight-list", "");
    CHECK_EQ(af->_parseWeightList(3).size(), 0);

    /** @arg a normal list is parsed and normalized to a sum of 1 */
    af->setParam<string>("weight-list", "1,2,1");
    auto w = af->_parseWeightList(3);
    REQUIRE_EQ(w.size(), 3);
    CHECK_EQ(w[0], doctest::Approx(0.25));
    CHECK_EQ(w[1], doctest::Approx(0.5));
    CHECK_EQ(w[2], doctest::Approx(0.25));

    /** @arg a size mismatch falls back to the equal weight (empty result) */
    af->setParam<string>("weight-list", "1,2");
    CHECK_EQ(af->_parseWeightList(3).size(), 0);

    /** @arg "nan" is rejected (no longer bypasses the sum validation) -> empty result */
    af->setParam<string>("weight-list", "nan,1");
    CHECK_EQ(af->_parseWeightList(2).size(), 0);

    /** @arg "inf" is rejected -> empty result */
    af->setParam<string>("weight-list", "inf,1");
    CHECK_EQ(af->_parseWeightList(2).size(), 0);

    /** @arg an overflow literal "1e999" makes std::stod throw out_of_range, treated as 0 (never
     *  becoming inf); the remaining item keeps the list valid and finite */
    af->setParam<string>("weight-list", "1e999,1");
    auto ov = af->_parseWeightList(2);
    REQUIRE_EQ(ov.size(), 2);
    CHECK_EQ(ov[0], 0.0);
    CHECK_EQ(ov[1], doctest::Approx(1.0));

    /** @arg all-zero weights have a sum of 0 -> invalid, falls back to the equal weight */
    af->setParam<string>("weight-list", "0,0,0");
    CHECK_EQ(af->_parseWeightList(3).size(), 0);

    /** @arg an overflowing sum (each item finite but the sum is inf) normalizes every weight to 0
     *  and would clear the holdings, so the whole list is rejected */
    af->setParam<string>("weight-list", "1e308,1e308");
    CHECK_EQ(af->_parseWeightList(2).size(), 0);
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
