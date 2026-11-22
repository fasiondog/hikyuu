/*
 * test_MarketConfigCheck.cpp
 *
 *  Created on: 2026-11-22
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/plugin/MarketConfigCheck.h>

using namespace hku;

/**
 * @defgroup test_MarketConfigCheck test_MarketConfigCheck
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_checkMarketConfig") {
    /** @arg The normal CN / US / 24h registrations are accepted */
    CHECK_UNARY(
      checkMarketConfig("SH", "Shanghai", "CN market", "000001", 930, 1130, 1300, 1500, 480, 0, 0));
    CHECK_UNARY(
      checkMarketConfig("NASDAQ", "NASDAQ", "US market", "NDX", 930, 1600, 0, 0, -300, 1, 1));
    CHECK_UNARY(checkMarketConfig("BINANCE", "Binance", "crypto", "BTCUSDT", 0, 0, 0, 0, 0, 0, 0));

    /** @arg The invalid timezone fields are rejected */
    CHECK_UNARY_FALSE(
      checkMarketConfig("TST", "test", "test", "000001", 930, 1130, 1300, 1500, 900, 0, 0));
    CHECK_UNARY_FALSE(
      checkMarketConfig("TST", "test", "test", "000001", 930, 1130, 1300, 1500, 480, 3, 0));
    CHECK_UNARY_FALSE(
      checkMarketConfig("TST", "test", "test", "000001", 930, 1130, 1300, 1500, 480, 0, -1));

    /** @arg The HHMM input is validated: neither the hour nor the minute may overflow */
    CHECK_UNARY_FALSE(
      checkMarketConfig("TST", "test", "test", "000001", 960, 1130, 1300, 1500, 480, 0, 0));
    CHECK_UNARY_FALSE(
      checkMarketConfig("TST", "test", "test", "000001", 930, 1130, 1300, 2400, 480, 0, 0));

    /** @arg A 24h market can not carry a daylight rule (the repeated hour has no inverse) */
    CHECK_UNARY_FALSE(checkMarketConfig("TST", "test", "test", "000001", 0, 0, 0, 0, -300, 1, 0));

    /** @arg An empty market identifier is rejected */
    CHECK_UNARY_FALSE(
      checkMarketConfig("", "test", "test", "000001", 930, 1130, 1300, 1500, 480, 0, 0));
}

/** @} */
