/*
 * test_SYS_Simple.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/portfolio/crt/PF_Simple.h>

using namespace hku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 Portfolio基础操作 */
TEST_CASE("test_PF_for_base") {
    PortfolioPtr pf = PF_Simple();
    CHECK_EQ(pf->name(), "PF_Simple");

    /** @arg 克隆操作 */
    PFPtr pf2 = pf->clone();
    CHECK_NE(pf2.get(), pf.get());
    CHECK_EQ(pf2->name(), pf->name());
}

/** @} */
