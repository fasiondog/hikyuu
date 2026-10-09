/*
 * test_PositionRecord.cpp
 *
 *  Created on: 2026-10-9
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/trade_manage/PositionRecord.h>

using namespace hku;

/**
 * @defgroup test_PositionRecord test_PositionRecord
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points: totalProfit is NaN for unclosed records, exact for closed ones */
TEST_CASE("test_PositionRecord_totalProfit") {
    PositionRecord p;
    p.buyMoney = 1000.0;
    p.sellMoney = 1200.0;
    p.totalCost = 20.0;

    /** @arg an unclosed record returns Null (NaN), not 0 */
    p.cleanDatetime = Null<Datetime>();
    CHECK_UNARY(std::isnan(p.totalProfit()));

    /** @arg a genuine zero profit is still distinguishable (0, not NaN) */
    p.cleanDatetime = Datetime(199911170000);
    p.sellMoney = p.buyMoney + p.totalCost;
    CHECK_UNARY(!std::isnan(p.totalProfit()));
    CHECK_EQ(p.totalProfit(), 0.0);

    /** @arg a closed record: sell - buy - cost */
    p.sellMoney = 1200.0;
    CHECK_EQ(p.totalProfit(), 180.0);
}

/** @} */
