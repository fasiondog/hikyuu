/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/slippage/crt/SP_LogNormal.h>

using namespace hku;

/**
 * @defgroup test_Slippage test_Slippage
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_LogNormalSlippage") {
    auto sp = SP_LogNormal(0.0, 0.1);
    CHECK_EQ(sp->name(), "SP_LogNormal");

    CHECK_GE(sp->getRealBuyPrice(Datetime(202201010930), 10.0), 10.0);
    CHECK_LE(sp->getRealSellPrice(Datetime(202201010930), 10.0), 10.0);
}

TEST_CASE("test_LogNormalSlippage_seed") {
    /** @arg the same seed produces identical sequences across instances (after reset) */
    auto sp1 = SP_LogNormal(0.0, 0.1);
    sp1->setParam<int64_t>("seed", 42);
    auto sp2 = SP_LogNormal(0.0, 0.1);
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
    auto sp3 = SP_LogNormal(0.0, 0.1);
    sp3->setParam<int64_t>("seed", 43);
    sp3->reset();
    CHECK_NE(sp3->getRealBuyPrice(Datetime(202201010930), 10.0), first);
}

TEST_CASE("test_LogNormalSlippage_adverse_only_false") {
    /** @arg adverse_only=false produces both favorable and unfavorable offsets */
    auto sp = SP_LogNormal(0.0, 0.1, false);
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
    auto a = SP_LogNormal(0.0, 0.1, false);
    auto b = SP_LogNormal(0.0, 0.1, false);
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
    auto sp2 = SP_LogNormal(0.0, 0.1);
    sp2->setParam<int64_t>("seed", 42);
    sp2->reset();
    for (int i = 0; i < 200; ++i) {
        CHECK_GE(sp2->getRealBuyPrice(Datetime(202201010930), 10.0), 10.0);
        CHECK_LE(sp2->getRealSellPrice(Datetime(202201010930), 10.0), 10.0);
    }
}

/** @par Test points */
TEST_CASE("test_LogNormalSlippage_short_side") {
    const Datetime dt(202201010930);
    const size_t total = 20000;

    /** @arg a wider short stddev makes the short-side slippage larger */
    auto sp = SP_LogNormal(0.0, 0.01, true, 0.0, 0.5);
    sp->setParam<int64_t>("seed", 42);
    sp->reset();
    double long_sum = 0.0, short_sum = 0.0;
    for (size_t i = 0; i < total; ++i) {
        long_sum += sp->getRealBuyPrice(dt, 10.0) - 10.0;
        short_sum += sp->getRealBuyShortPrice(dt, 10.0) - 10.0;
    }
    CHECK_GT(short_sum, long_sum * 3.0);

    /** @arg without short parameters both sides draw from the same distribution */
    auto sp2 = SP_LogNormal(0.0, 0.05);
    sp2->setParam<int64_t>("seed", 7);
    sp2->reset();
    auto sp3 = SP_LogNormal(0.0, 0.05);
    sp3->setParam<int64_t>("seed", 7);
    sp3->reset();
    for (int i = 0; i < 20; ++i) {
        CHECK_EQ(sp2->getRealBuyShortPrice(dt, 10.0), sp3->getRealBuyPrice(dt, 10.0));
    }
}

/** @} */
