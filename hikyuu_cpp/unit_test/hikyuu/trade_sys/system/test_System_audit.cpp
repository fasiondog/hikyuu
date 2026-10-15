/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-15
 *      Author: fasiondog
 *
 *  Regression tests for the System audit fixes.
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>

using namespace hku;

/**
 * @defgroup test_System_audit test_System_audit
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Check point: ignore_sell_sg suppresses the SG sell signal (the parameter used to be
 * registered and documented but never read) */
TEST_CASE("test_System_ignore_sell_sg") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    TMPtr tm = crtTM(Datetime(199001010000LL), 100000.0, TC_Zero());
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));

    // With no stop-loss / take-profit / goal set, the SG is the only exit source
    auto sg_sell_count = [&](bool ignore) {
        auto sys = SYS_Simple();
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->setParam<bool>("ignore_sell_sg", ignore);
        sys->setTM(tm->clone());
        sys->setSG(sg->clone());
        sys->setMM(MM_FixedCount(100));
        sys->run(stk, query);
        size_t cnt = 0;
        for (const auto& tr : sys->getTM()->getTradeList()) {
            if (tr.business == BUSINESS_SELL && tr.from == PART_SIGNAL) {
                cnt++;
            }
        }
        return cnt;
    };

    /** @arg default: the SG sell signal liquidates the position */
    CHECK_UNARY(sg_sell_count(false) > 0);
    /** @arg ignore_sell_sg=true: the SG sell signal is not acted on */
    CHECK_EQ(sg_sell_count(true), 0);
}

/** @par Check point: max_delay_count=0 still allows one delayed execution (the request used to be
 * dropped on the next open because the counter started at 1, so a value of 0 silently disabled the
 * delayed trading) */
TEST_CASE("test_System_runMoment_max_delay_count_zero") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 2);

    auto sys = SYS_Simple(crtTM(Datetime(199001010000LL), 100000.0), MM_FixedCount(100),
                          EnvironmentPtr(), ConditionPtr(), SG_AllwaysBuy());
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<int>("max_delay_count", 0);
    sys->setTO(kdata);
    sys->readyForRun();

    /** @arg bar 0: the buy is delayed, no trade is made and one pending request is visible */
    MomentResult r0 = sys->runMoment(kdata[0].datetime);
    CHECK_UNARY(r0.tradesOnOpen.empty());
    CHECK_UNARY(r0.tradesOnClose.empty());
    REQUIRE_EQ(r0.delayOnNextOpen.size(), 1);

    /** @arg bar 1: the single allowed delay is executed at the open */
    MomentResult r1 = sys->runMoment(kdata[1].datetime);
    CHECK_UNARY(!r1.tradesOnOpen.empty());
}

/** @} */
