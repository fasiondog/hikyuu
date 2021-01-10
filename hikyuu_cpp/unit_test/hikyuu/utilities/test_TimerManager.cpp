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
    CHECK(tm.addDelayFunc(Seconds(1), []() { HKU_INFO("test delay *************************"); }));
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

/** @} */