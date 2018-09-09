/*
 * global.cpp
 *
 *  Created on: 2011-2-20
 *      Author: fasiondog
 */

#include <iostream>
#include <chrono>
#include <boost/test/unit_test.hpp>
#include <hikyuu/hikyuu.h>

#ifdef __WIN32__
#include <windows.h>
#endif

using namespace hku;

/**
 * 全局夹具，初始化
 */
struct GConfig {
    GConfig();

    ~GConfig();

    std::chrono::system_clock::time_point start_time;
};


