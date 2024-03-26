/*
 * test_AllocateFunds.cpp
 *
 *  Created on: 2018-2-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>

using namespace hku;

/**
 * @defgroup test_AllocateFunds test_AllocateFunds
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AllocateFunds") {
    StockManager& sm = StockManager::instance();

    SEPtr se = SE_Fixed();
    AFPtr af = AF_EqualWeight();
    TMPtr tm = crtTM(Datetime(200101010000L), 100000);
    TMPtr subtm = crtTM(Datetime(200101010000L), 0);
    SYSPtr sys = SYS_Simple(subtm->clone());

    SystemWeightList se_list;
    SystemList hold_list;
    SystemList ac_list;
    SystemWeightList sw_list;

    /** @arg 测试setTM */
    CHECK_UNARY(!af->getTM());
    af->setTM(tm);
    CHECK_UNARY(af->getTM());

    /** @arg 测试clone */
    AFPtr af2 = af->clone();
    CHECK_NE(af2.get(), af.get());
    CHECK_EQ(af2->name(), af->name());

    //----------------------------------------------------
    // 测试对计划权重的调整
    //----------------------------------------------------
    /** @arg 不自动调整 auto_adjust = false  */
    SystemWeightList sw(10);
    SystemWeightList expect_sw(10);
    for (size_t i = 0; i < 10; i++) {
        sw[i].weight = i;
        expect_sw[i].weight = 9 - i;
    }

    AllocateFundsBase::adjustWeight(sw, 1.0, false, false);
    CHECK_EQ(sw.size(), 9);
    for (size_t i = 0; i < 9; i++) {
        CHECK_EQ(sw[i].weight, expect_sw[i].weight);
    }

    /** @arg auto_adjust = true, ignore_zero = true*/
    sw.clear();
    sw.emplace_back(SYSPtr(), 0.0);
    sw.emplace_back(SYSPtr(), 2.0);
    sw.emplace_back(SYSPtr(), Null<price_t>());
    sw.emplace_back(SYSPtr(), 3.0);
    expect_sw.clear();
    expect_sw.emplace_back(SYSPtr(), 3.0 / 5.0 * 0.8);
    expect_sw.emplace_back(SYSPtr(), 2.0 / 5.0 * 0.8);

    AllocateFundsBase::adjustWeight(sw, 0.8, true, true);
    CHECK_EQ(sw.size(), expect_sw.size());
    CHECK_EQ(sw[0].weight, doctest::Approx(expect_sw[0].weight));
    CHECK_EQ(sw[1].weight, doctest::Approx(expect_sw[1].weight));

    /** @arg auto_adjust = true, ignore_zero = false*/
    sw.clear();
    sw.emplace_back(SYSPtr(), 0.0);
    sw.emplace_back(SYSPtr(), 2.0);
    sw.emplace_back(SYSPtr(), Null<price_t>());
    sw.emplace_back(SYSPtr(), 3.0);
    expect_sw.clear();
    expect_sw.emplace_back(SYSPtr(), 3.0 / 5.0 * (2.0 / 4.0) * 0.8);
    expect_sw.emplace_back(SYSPtr(), 2.0 / 5.0 * (2.0 / 4.0) * 0.8);

    AllocateFundsBase::adjustWeight(sw, 0.8, true, false);
    CHECK_EQ(sw.size(), expect_sw.size());
    CHECK_EQ(sw[0].weight, doctest::Approx(expect_sw[0].weight));
    CHECK_EQ(sw[1].weight, doctest::Approx(expect_sw[1].weight));
}

/** @} */
