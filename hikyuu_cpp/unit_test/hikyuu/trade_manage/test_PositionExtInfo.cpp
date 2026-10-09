/*
 * test_PositionExtInfo.cpp
 *
 *  Created on: 2026-10-9
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/trade_manage/PositionExtInfo.h>

using namespace hku;

/**
 * @defgroup test_PositionExtInfo test_PositionExtInfo
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points: currentPullBack guards a zero reference price */
TEST_CASE("test_PositionExtInfo_currentPullBack") {
    PositionExtInfo info;

    /** @arg default (zero) max prices yield 0 instead of NaN/Inf */
    CHECK_UNARY(std::isfinite(info.currentPullBack1()));
    CHECK_UNARY(std::isfinite(info.currentPullBack2()));
    CHECK_EQ(info.currentPullBack1(), 0.0);
    CHECK_EQ(info.currentPullBack2(), 0.0);

    /** @arg a normal peak with a lower current price still clamps to <= 0 */
    info.maxClosePrice = 10.0;
    info.maxHighPrice = 10.0;
    info.currentClosePrice = 8.0;
    CHECK_UNARY(info.currentPullBack1() <= 0.0);
    CHECK_UNARY(info.currentPullBack2() <= 0.0);
}

/** @} */
