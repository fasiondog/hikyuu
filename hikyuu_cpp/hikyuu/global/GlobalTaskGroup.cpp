/*
 * GlobalTaskGroup.cpp
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-4-20
 *      Author: fasiondog
 */

#include "hikyuu/GlobalInitializer.h"
#include "hikyuu/utilities/Log.h"
#include "GlobalTaskGroup.h"
#include "hikyuu/StockManager.h"

namespace hku {

static TaskGroup* g_threadPool;

TaskGroup* getGlobalTaskGroup() {
    static std::once_flag oc;
    std::call_once(oc, [&]() {
        // auto cpu_num = std::thread::hardware_concurrency();
        // if (cpu_num >= 4) {
        //     cpu_num -= 2;
        // } else if (cpu_num > 1) {
        //     cpu_num--;
        // }
        const auto& param = StockManager::instance().getHikyuuParameter();
        size_t worker_num =
          param.tryGet<int>("commont_thread_pool_num", std::thread::hardware_concurrency());
        HKU_INFO("work_num: {}", worker_num);
        g_threadPool = new TaskGroup(worker_num);
    });
    return g_threadPool;
}

void releaseGlobalTaskGroup() {
    HKU_TRACE("releaseGlobalTaskGroup");
    if (g_threadPool) {
        g_threadPool->stop();
        delete g_threadPool;
        g_threadPool = nullptr;
    }
}

} /* namespace hku */