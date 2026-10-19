/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/slippage/crt/SP_Normal.h>

using namespace hku;

/**
 * @defgroup test_Slippage test_Slippage
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_NormalSlippage") {
    auto sp = SP_Normal(0.0, 0.1);
    CHECK_EQ(sp->name(), "SP_Normal");

    CHECK_GE(sp->getRealBuyPrice(Datetime(202201010930), 10.0), 10.0);
    CHECK_LE(sp->getRealSellPrice(Datetime(202201010930), 10.0), 10.0);
}

TEST_CASE("test_NormalSlippage_seed") {
    /** @arg the same seed produces identical sequences across instances (after reset) */
    auto sp1 = SP_Normal(0.0, 0.1);
    sp1->setParam<int64_t>("seed", 42);
    auto sp2 = SP_Normal(0.0, 0.1);
    sp2->setParam<int64_t>("seed", 42);
    sp1->reset();
    sp2->reset();
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(sp1->getRealBuyPrice(Datetime(202201010930), 10.0),
                 sp2->getRealBuyPrice(Datetime(202201010930), 10.0));
        CHECK_EQ(sp1->getRealSellPrice(Datetime(202201010930), 10.0),
                 sp2->getRealSellPrice(Datetime(202201010930), 10.0));
    }

    /** @arg reset replays the same sequence from the seed */
    sp1->reset();
    price_t first = sp1->getRealBuyPrice(Datetime(202201010930), 10.0);
    for (int i = 0; i < 3; ++i) {
        sp1->getRealBuyPrice(Datetime(202201010930), 10.0);
    }
    sp1->reset();
    CHECK_EQ(sp1->getRealBuyPrice(Datetime(202201010930), 10.0), first);

    /** @arg a different seed produces a different sequence */
    auto sp3 = SP_Normal(0.0, 0.1);
    sp3->setParam<int64_t>("seed", 43);
    sp3->reset();
    CHECK_NE(sp3->getRealBuyPrice(Datetime(202201010930), 10.0), first);
}

TEST_CASE("test_NormalSlippage_checkParam") {
    auto sp = SP_Normal(0.0, 0.05);
    /** @arg a nan mean is rejected */
    CHECK_THROWS_AS(sp->setParam("mean", Null<double>()), std::exception);
}

namespace {
// E|X| of X ~ N(mean, stddev): the sampled value is folded to a non-negative amplitude, so this
// folded expectation (not the mean) is the actual expected slippage of SP_Normal
double expectedAbsNormal(double mean, double stddev) {
    const double sqrt_2_pi = std::sqrt(2.0 / std::acos(-1.0));
    return stddev * sqrt_2_pi * std::exp(-mean * mean / (2.0 * stddev * stddev)) +
           mean * std::erf(mean / (stddev * std::sqrt(2.0)));
}
}  // namespace

/** @par Test points */
TEST_CASE("test_NormalSlippage_expected_slippage") {
    const size_t total = 100000;
    const Datetime datetime(202201010930);
    const price_t plan = 10.0;

    /** @arg mean = 0: the expected slippage converges to stddev * sqrt(2/pi) (~ 0.7979 * stddev),
     *  on both the buy and the sell side */
    auto sp1 = SP_Normal(0.0, 0.05);
    double sum_buy = 0.0, sum_sell = 0.0;
    for (size_t i = 0; i < total; ++i) {
        sum_buy += sp1->getRealBuyPrice(datetime, plan) - plan;
        sum_sell += plan - sp1->getRealSellPrice(datetime, plan);
    }
    const double expected1 = expectedAbsNormal(0.0, 0.05);
    CHECK_LE(std::abs(sum_buy / total - expected1), expected1 * 0.01);
    CHECK_LE(std::abs(sum_sell / total - expected1), expected1 * 0.01);

    /** @arg mean comparable to stddev: the expectation follows the folded-normal formula */
    auto sp2 = SP_Normal(0.05, 0.05);
    double sum2 = 0.0;
    for (size_t i = 0; i < total; ++i) {
        sum2 += sp2->getRealBuyPrice(datetime, plan) - plan;
    }
    const double expected2 = expectedAbsNormal(0.05, 0.05);
    CHECK_LE(std::abs(sum2 / total - expected2), expected2 * 0.01);
}

/** @par Test points */
TEST_CASE("test_NormalSlippage_adverse_only_false") {
    /** @arg adverse_only=false produces both favorable and unfavorable offsets */
    auto sp = SP_Normal(0.0, 0.1, false);
    sp->setParam<int64_t>("seed", 123);
    sp->reset();
    bool has_above = false, has_below = false;
    for (int i = 0; i < 200; ++i) {
        price_t buy = sp->getRealBuyPrice(Datetime(202201010930), 10.0);
        if (buy > 10.0)
            has_above = true;
        if (buy < 10.0)
            has_below = true;
    }
    CHECK_UNARY(has_above);
    CHECK_UNARY(has_below);

    /** @arg the same draw shifts the buy and the sell by the opposite signed offset */
    auto a = SP_Normal(0.0, 0.1, false);
    auto b = SP_Normal(0.0, 0.1, false);
    a->setParam<int64_t>("seed", 7);
    b->setParam<int64_t>("seed", 7);
    a->reset();
    b->reset();
    for (int i = 0; i < 20; ++i) {
        price_t buy = a->getRealBuyPrice(Datetime(202201010930), 10.0);
        price_t sell = b->getRealSellPrice(Datetime(202201010930), 10.0);
        CHECK_LE(std::abs((buy - 10.0) + (sell - 10.0)), 1e-12);
    }

    /** @arg adverse_only=true (default) only produces unfavorable offsets on buy */
    auto sp2 = SP_Normal(0.0, 0.1);
    sp2->setParam<int64_t>("seed", 123);
    sp2->reset();
    for (int i = 0; i < 200; ++i) {
        CHECK_GE(sp2->getRealBuyPrice(Datetime(202201010930), 10.0), 10.0);
    }
}

/** @par Test points */
TEST_CASE("test_NormalSlippage_short_side") {
    const Datetime dt(202201010930);

    /** @arg a zero stddev makes the draw equal to the mean */
    auto sp = SP_Normal(0.02, 0.0, true, 0.08, 0.0);
    CHECK(sp->getRealBuyShortPrice(dt, 10.0) == doctest::Approx(10.08));
    CHECK(sp->getRealSellShortPrice(dt, 10.0) == doctest::Approx(9.92));

    /** @arg the long side is untouched by the short parameters */
    CHECK(sp->getRealBuyPrice(dt, 10.0) == doctest::Approx(10.02));
    CHECK(sp->getRealSellPrice(dt, 10.0) == doctest::Approx(9.98));

    /** @arg without short parameters the short side equals the long side */
    auto sp2 = SP_Normal(0.02, 0.0);
    CHECK(sp2->getRealBuyShortPrice(dt, 10.0) == doctest::Approx(10.02));
    CHECK(sp2->getRealSellShortPrice(dt, 10.0) == doctest::Approx(9.98));

    /** @arg a negative short_stddev is rejected */
    CHECK_THROWS_AS(SP_Normal(0.0, 0.05, true, 0.0, -0.01), std::exception);
}

/** @} */
