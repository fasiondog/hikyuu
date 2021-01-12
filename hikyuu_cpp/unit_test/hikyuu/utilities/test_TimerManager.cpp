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

static void hello_test() {
    HKU_TRACE("hello");
}

TEST_CASE("test_TimerManager") {
    TimerManager tm;
    tm.start();

    CHECK_THROWS_AS(tm.addDurationFunc(1, TimeDelta(), hello_test), hku::exception);
    CHECK_THROWS_AS(tm.addDurationFunc(0, TimeDelta(1), hello_test), hku::exception);
    CHECK_THROWS_AS(tm.addDurationFunc(-1, TimeDelta(1), hello_test), hku::exception);

    CHECK_THROWS_AS(tm.addDelayFunc(TimeDelta(0), hello_test), hku::exception);
    CHECK_THROWS_AS(tm.addDelayFunc(TimeDelta(-1), hello_test), hku::exception);

    CHECK_THROWS_AS(tm.addFuncAtTime(Datetime::min(), hello_test), hku::exception);

    CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(TimeDelta(-1), hello_test), hku::exception);
    CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(TimeDelta(1), hello_test), hku::exception);

    CHECK_THROWS_AS(
      tm.addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), TimeDelta(-1), hello_test),
      hku::exception);
    CHECK_THROWS_AS(
      tm.addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), TimeDelta(1), hello_test),
      hku::exception);
    CHECK_THROWS_AS(
      tm.addFuncAtTimeEveryDay(Datetime::max(), Datetime::min(), TimeDelta(0, 1), hello_test),
      hku::exception);
    CHECK_THROWS_AS(
      tm.addFuncAtTimeEveryDay(Datetime::min(), Datetime(), TimeDelta(0, 1), hello_test),
      hku::exception);
    CHECK_THROWS_AS(
      tm.addFuncAtTimeEveryDay(Datetime(), Datetime::max(), TimeDelta(0, 1), hello_test),
      hku::exception);

    /*
    tm.addDelayFunc(Microseconds(30), []() { HKU_INFO("1 test delay *************************"); });
    tm.addDelayFunc(Seconds(2), []() { HKU_INFO("2 test delay *************************"); });
    tm.addDurationFunc(3, Milliseconds(500),
                       []() { HKU_INFO("3 test delay *************************"); });
    tm.addFuncAtTime(Datetime(202101112305),
                     []() { HKU_INFO("addFuncAtTime test delay *************************"); });
    tm.addFuncAtTimeEveryDay(Datetime::today(), Datetime::max(), TimeDelta(0, 23, 17), []() {
        HKU_INFO("addFuncAtTimeEveryDay test delay *************************");
    });
    tm.addFuncAtTime(Datetime::today(), Datetime::max(), Datetime(202101122300),
                     []() { HKU_INFO("addFuncAtTime test delay *************************"); });
    std::this_thread::sleep_for(std::chrono::seconds(60));*/
}

/** @} */