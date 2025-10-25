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

/** @par 检测点 */
TEST_CASE("test_LogNormalSlippage") {
    auto sp = SP_LogNormal(0.0, 0.1);
    CHECK_EQ(sp->name(), "SP_LogNormal");

    CHECK_GE(sp->getRealBuyPrice(Datetime(202201010930), 10.0), 10.0);
    CHECK_LE(sp->getRealSellPrice(Datetime(202201010930), 10.0), 10.0);
}

/** @} */
