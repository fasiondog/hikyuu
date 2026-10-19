/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/slippage/crt/SP_TruncNormal.h>

using namespace hku;

/**
 * @defgroup test_Slippage test_Slippage
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SP_TruncNormal") {
    auto sp = SP_TruncNormal(0.0, 0.1);
    CHECK_EQ(sp->name(), "SP_TruncNormal");

    CHECK_GE(sp->getRealBuyPrice(Datetime(202201010930), 10.0), 10.0);
    CHECK_LE(sp->getRealSellPrice(Datetime(202201010930), 10.0), 10.0);
}

TEST_CASE("test_TruncNormalSlippage_seed") {
    /** @arg the same seed produces identical sequences across instances (after reset) */
    auto sp1 = SP_TruncNormal(0.0, 0.05, -0.1, 0.1);
    sp1->setParam<int64_t>("seed", 42);
    auto sp2 = SP_TruncNormal(0.0, 0.05, -0.1, 0.1);
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
    auto sp3 = SP_TruncNormal(0.0, 0.05, -0.1, 0.1);
    sp3->setParam<int64_t>("seed", 43);
    sp3->reset();
    CHECK_NE(sp3->getRealBuyPrice(Datetime(202201010930), 10.0), first);
}

TEST_CASE("test_TruncNormalSlippage_far_range") {
    /** @arg a truncated range far from the mean terminates and clamps into the range */
    auto sp = SP_TruncNormal(0.0, 0.01, 0.05, 0.08);
    for (int i = 0; i < 100; ++i) {
        price_t value = sp->getRealBuyPrice(Datetime(202201010930), 10.0) - 10.0;
        CHECK_GE(value, 0.05 - 1e-12);
        CHECK_LE(value, 0.08 + 1e-12);
    }
}

TEST_CASE("test_TruncNormalSlippage_adverse_only_false") {
    /** @arg adverse_only=false produces both favorable and unfavorable offsets */
    auto sp = SP_TruncNormal(0.0, 0.05, -0.1, 0.1, false);
    sp->setParam<int64_t>("seed", 42);
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
    auto a = SP_TruncNormal(0.0, 0.05, -0.1, 0.1, false);
    auto b = SP_TruncNormal(0.0, 0.05, -0.1, 0.1, false);
    a->setParam<int64_t>("seed", 7);
    b->setParam<int64_t>("seed", 7);
    a->reset();
    b->reset();
    for (int i = 0; i < 20; ++i) {
        price_t buy = a->getRealBuyPrice(Datetime(202201010930), 10.0);
        price_t sell = b->getRealSellPrice(Datetime(202201010930), 10.0);
        CHECK_LE(std::abs((buy - 10.0) + (sell - 10.0)), 1e-12);
    }

    /** @arg adverse_only=true (default) only produces unfavorable offsets */
    auto sp2 = SP_TruncNormal(0.0, 0.05, -0.1, 0.1);
    sp2->setParam<int64_t>("seed", 42);
    sp2->reset();
    for (int i = 0; i < 200; ++i) {
        CHECK_GE(sp2->getRealBuyPrice(Datetime(202201010930), 10.0), 10.0);
        CHECK_LE(sp2->getRealSellPrice(Datetime(202201010930), 10.0), 10.0);
    }
}

/** @par Test points */
TEST_CASE("test_TruncNormalSlippage_short_side") {
    const Datetime dt(202201010930);

    /** @arg a degenerate short truncation interval makes the short draw deterministic */
    auto sp = SP_TruncNormal(0.0, 0.05, -0.1, 0.1, true, 0.0, 0.05, 0.5, 0.5);
    CHECK(sp->getRealBuyShortPrice(dt, 10.0) == doctest::Approx(10.5));
    CHECK(sp->getRealSellShortPrice(dt, 10.0) == doctest::Approx(9.5));

    /** @arg the long side keeps its own interval */
    for (int i = 0; i < 100; ++i) {
        price_t off = sp->getRealBuyPrice(dt, 10.0) - 10.0;
        CHECK_GE(off, 0.0);
        CHECK_LE(off, 0.1 + 1e-9);
    }

    /** @arg without short parameters the short amplitude stays inside the long interval */
    auto sp2 = SP_TruncNormal(0.0, 0.05, 0.0, 0.1);
    for (int i = 0; i < 100; ++i) {
        CHECK_GE(sp2->getRealBuyShortPrice(dt, 10.0), 10.0);
        CHECK_LE(sp2->getRealBuyShortPrice(dt, 10.0), 10.1 + 1e-9);
    }

    /** @arg an all-negative interval is legal on both sides */
    CHECK_NOTHROW(SP_TruncNormal(0.0, 0.05, -0.4, -0.2, false));
    CHECK_NOTHROW(SP_TruncNormal(0.0, 0.05, 0.2, 0.4, true, 0.0, 0.05, -0.4, -0.2));

    /** @arg setting one bound alone after construction is still validated against the other */
    auto sp3 = SP_TruncNormal(0.0, 0.05, -0.1, 0.1);
    CHECK_THROWS_AS(sp3->setParam("short_max_value", -0.5), std::exception);
}

/** @} */
