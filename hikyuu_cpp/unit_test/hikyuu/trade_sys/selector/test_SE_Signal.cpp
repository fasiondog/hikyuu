/*
 * test_SE_Signal.cpp
 *
 *  Created on: 2026-10-24
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/trade_sys/selector/crt/SE_Signal.h>

using namespace hku;

/**
 * @defgroup test_SE_Signal test_SE_Signal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SE_Signal_name") {
    SEPtr se = SE_Signal();

    /** @arg The default name is SE_Signal (the old imp used the typo SE_Sigal) */
    CHECK_EQ(se->name(), "SE_Signal");

    /** @arg The clone keeps the corrected name */
    CHECK_EQ(se->clone()->name(), "SE_Signal");
}

/** @} */
