/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 *
 *  Regression tests for the System::runMoment result contract.
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crt/crtTM.h>

using namespace hku;

/**
 * @defgroup test_System_runMoment test_System_runMoment
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Check point: with buy_delay, the runMoment pending request appears in delayOnNextOpen
 * (previously this field had no filling point) */
TEST_CASE("test_System_runMoment_delay_request") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 0);

    auto sys = SYS_Simple(crtTM(Datetime(199001010000LL), 100000.0), MM_Nothing(), EnvironmentPtr(),
                          ConditionPtr(), SG_AllwaysBuy());
    sys->setParam<bool>("buy_delay", true);
    sys->setTO(kdata);
    sys->readyForRun();

    /** @arg delayed buy: no immediate trade, the pending request is visible */
    MomentResult r = sys->runMoment(kdata[0].datetime);
    CHECK_UNARY(r.tradesOnOpen.empty());
    CHECK_UNARY(r.tradesOnClose.empty());
    REQUIRE_EQ(r.delayOnNextOpen.size(), 1);
    CHECK_UNARY(r.delayOnNextOpen[0].valid);
    CHECK_EQ(r.delayOnNextOpen[0].business, BUSINESS_BUY);
}

/** @} */
