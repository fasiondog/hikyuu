/*
 * test_ST_Saftyloss.cpp
 *
 *  Created on: 2026-10-21
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <limits>
#include <hikyuu/trade_sys/stoploss/crt/ST_Saftyloss.h>

using namespace hku;

/**
 * @defgroup test_ST_Saftyloss test_ST_Saftyloss
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ST_Saftyloss_invalid_p") {
    /** @arg p must be greater than 0, otherwise the stop-loss line is above the price */
    CHECK_THROWS_AS(ST_Saftyloss(10, 3, -1.0), std::exception);
    CHECK_THROWS_AS(ST_Saftyloss(10, 3, 0.0), std::exception);

    /** @arg p must be finite, a non-finite p discards every bar and silently disables the stop */
    CHECK_THROWS_AS(ST_Saftyloss(10, 3, std::numeric_limits<double>::quiet_NaN()), std::exception);
    CHECK_THROWS_AS(ST_Saftyloss(10, 3, std::numeric_limits<double>::infinity()), std::exception);

    /** @arg a valid p is accepted */
    CHECK_NOTHROW(ST_Saftyloss(10, 3, 0.5));
    CHECK_NOTHROW(ST_Saftyloss());
}

/** @} */
