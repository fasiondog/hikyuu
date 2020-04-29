/*
 * test_iniparser.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2010-5-26
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/thread/ThreadPool.h>
#include <hikyuu/utilities/SpendTimer.h>
#include <hikyuu/Log.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_ThreadPool test_hikyuu_ThreadPool
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ThreadPool") {
    {
        SPEND_TIME(test_temp);
        ThreadPool tg;
        for (int i = 0; i < 100; i++) {
            tg.submit([=]() { fmt::print("{}: ----------------------\n", i); });
        }
        tg.join();
    }
}

/** @} */