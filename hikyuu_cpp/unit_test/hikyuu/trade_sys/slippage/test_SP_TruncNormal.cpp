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

/** @} */
