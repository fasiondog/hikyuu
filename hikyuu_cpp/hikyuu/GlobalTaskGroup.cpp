/*
 * GlobalTaskGroup.cpp
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-4-20
 *      Author: fasiondog
 */

#include <chrono>
#include "GlobalTaskGroup.h"

namespace hku {

static ThreadPool* g_threadPool;

HKU_API ThreadPool* getGlobalTaskGroup() {
    return g_threadPool;
}

void initThreadPool() {
    if (!g_threadPool) {
        auto cpu_num = std::thread::hardware_concurrency();
        if (cpu_num >= 4) {
            cpu_num -= 2;
        } else if (cpu_num > 1) {
            cpu_num--;
        }
        g_threadPool = new ThreadPool(cpu_num);
    }
}

void releaseThreadPool() {
    if (g_threadPool) {
        g_threadPool->stop();
        delete g_threadPool;
        g_threadPool = nullptr;
    }
}

} /* namespace hku */