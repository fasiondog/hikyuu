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

namespace hku {

static std::shared_ptr<TaskGroup> g_threadPool;

std::shared_ptr<TaskGroup>& getGlobalTaskGroup() {
    static std::once_flag oc;
    std::call_once(oc, [&]() {
        auto cpu_num = std::thread::hardware_concurrency();
        if (cpu_num >= 4) {
            cpu_num -= 2;
        } else if (cpu_num > 1) {
            cpu_num--;
        }
        g_threadPool = std::make_shared<TaskGroup>(cpu_num);
    });
    return g_threadPool;
}

void releaseGlobalTaskGroup() {
    HKU_TRACE("releaseGlobalTaskGroup");
    if (g_threadPool) {
        g_threadPool->stop();
        g_threadPool.reset();
    }
}

} /* namespace hku */