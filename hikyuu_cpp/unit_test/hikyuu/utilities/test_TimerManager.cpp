/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-09
 *     Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/TimerManager.h>
#include <hikyuu/Log.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_TimerManager test_hikyuu_TimerManager
 * @ingroup test_hikyuu_utilities
 * @{
 */

TEST_CASE("test_TimerManager") {
    TimerManager tm;
    tm.start();
    tm.addDelayFunc(Microseconds(30), []() { HKU_INFO("1 test delay *************************"); });
    tm.addDelayFunc(Seconds(2), []() { HKU_INFO("2 test delay *************************"); });
    tm.addDurationFunc(std::numeric_limits<int>::max(), Milliseconds(500),
                       []() { HKU_INFO("3 test delay *************************"); });
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

/** @} */