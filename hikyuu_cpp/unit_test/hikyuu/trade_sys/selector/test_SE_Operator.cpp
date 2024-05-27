/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_sys/selector/crt/SE_Operator.h>

using namespace hku;

/**
 * @defgroup test_Selector_operator test_Selector_operator
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SE_Add") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);
}

/** @} */