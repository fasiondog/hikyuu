/*
 * test_PF_for_sys_use_self_tm.cpp
 *
 *  Created on: 2026-9-27
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/portfolio/crt/PF_WithoutAF.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_CrossGold.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/EMA.h>

using namespace hku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test point: with sys_use_self_tm on, prototype systems carrying their own tm must still
 *  enter the selector pool instead of being silently dropped (ISS-025 regression) */
TEST_CASE("test_PF_for_sys_use_self_tm") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    sys->setSG(SG_CrossGold(EMA(CLOSE(), 12), EMA(CLOSE(), 26)));
    sys->setMM(MM_FixedCount(100));
    sys->setTM(crtTM(Datetime(199001010000L), 500000));

    StockList stocks = {sm["sz000001"], sm["sz000063"], sm["sz000651"]};
    for (auto& stk : stocks) {
        REQUIRE_FALSE(stk.isNull());
    }

    TMPtr tm = crtTM(Datetime(199001010000L), 500000);
    SEPtr se = SE_Fixed();
    se->addStockList(stocks, sys);
    PFPtr pf = PF_WithoutAF(tm, se, 1, "query", true, true, true);

    KQuery query = KQueryByDate(Datetime(201101010000L), Null<Datetime>(), KQuery::DAY);
    pf->run(query);

    /** @arg All systems with their own tm are calculated and selectable */
    CHECK_EQ(se->getSelected(Null<Datetime>()).size(), stocks.size());
}

/** @} */
