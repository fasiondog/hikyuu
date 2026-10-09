/*
 * test_crtFixedAValueTC.cpp
 *
 *  Created on: 2026-10-9
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/TC_FixedA2015.h>
#include <hikyuu/trade_manage/crt/TC_FixedA2017.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>

using namespace hku;

/**
 * @defgroup test_crtFixedAValueTC test_crtFixedAValueTC
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points: 2015 transfer fee is charged on the Shanghai market only */
TEST_CASE("test_TC_FixedA2015") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    Stock sz = sm.getStock("sz000001");
    CostRecord result, expect;
    TradeCostPtr cost_func = TC_FixedA2015(0.0018, 5, 0.001, 0.00002);
    const Datetime dt(201601010000);

    /** @arg Buy a Shanghai stock: commission + transfer fee */
    result = cost_func->getBuyCost(dt, sh, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.transferfee = 0.2;
    expect.total = 18.2;
    CHECK_EQ(result, expect);

    /** @arg Buy a Shenzhen stock: commission only (no transfer fee before 2017) */
    result = cost_func->getBuyCost(dt, sz, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.total = 18.0;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shenzhen stock: commission + stamp duty, no transfer fee */
    result = cost_func->getSellCost(dt, sz, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.total = 28.0;
    CHECK_EQ(result, expect);
}

/** @par Test points: 2017 transfer fee is charged on both markets, in both directions */
TEST_CASE("test_TC_FixedA2017") {
    StockManager& sm = StockManager::instance();
    Stock sz = sm.getStock("sz000001");
    CostRecord result, expect;
    TradeCostPtr cost_func = TC_FixedA2017(0.0018, 5, 0.001, 0.00002);
    const Datetime dt(201801010000);

    /** @arg Buy a Shenzhen stock: commission + transfer fee */
    result = cost_func->getBuyCost(dt, sz, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.transferfee = 0.2;
    expect.total = 18.2;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shenzhen stock: transfer fee is now charged in both directions (regression of
     * the previously sell-side-omitted bug) */
    result = cost_func->getSellCost(dt, sz, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.transferfee = 0.2;
    expect.total = 28.2;
    CHECK_EQ(result, expect);
}

/** @par Test points: TC_FixedA is the rolling current model — transfer fee 0.01‰ all markets, stamp
 * duty 0.5‰ */
TEST_CASE("test_TC_FixedA_current") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    Stock sz = sm.getStock("sz000001");
    CostRecord result, expect;
    TradeCostPtr cost_func = TC_FixedA();
    const Datetime dt(202401010000);

    /** @arg Buy a Shenzhen stock: transfer fee charged on all markets at 0.01‰ */
    result = cost_func->getBuyCost(dt, sz, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.transferfee = 0.1;
    expect.total = 18.1;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shanghai stock: stamp duty halved to 0.5‰ plus transfer fee */
    result = cost_func->getSellCost(dt, sh, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.stamptax = 5.0;
    expect.transferfee = 0.1;
    expect.total = 23.1;
    CHECK_EQ(result, expect);
}

/** @} */
