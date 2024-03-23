/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
// #include <hikyuu/trade_sys/allocatefunds/SystemWeight.h>
// #include<hikyuu / StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>

using namespace hku;

/**
 * @defgroup test_AllocateFunds test_SystemWeight
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点，不自动调仓 */
TEST_CASE("test_SystemWeight") {
    // SystemWeight w;
    // CHECK_EQ(w.getWeight(), 1.0);
    // // CHECK_THROWS(w.setWeight(1.1));
    // w.setWeight(0.9);
    // CHECK_EQ(w.getWeight(), 0.9);

    // TMPtr tm = crtTM(Datetime(200101010000L), 100000);
    // SYSPtr sys = SYS_Simple(tm);

    // CHECK_THROWS(SystemWeight(sys, 1.1));
    // HKU_INFO("{}", SystemWeight(sys, 0.2));
}

/** @} */