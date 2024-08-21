/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#include "hikyuu/GlobalInitializer.h"
#include <mutex>
#include "hikyuu/utilities/Log.h"
#include "hikyuu/global/GlobalTaskGroup.h"
#include "scheduler.h"

namespace hku {

static TimerManager *g_scheduler;

TimerManager *getScheduler() {
    static std::once_flag oc;
    // 使用内部公共任务组，减少内部线程
    std::call_once(oc, [&]() { g_scheduler = new TimerManager(getGlobalTaskGroup()); });
    return g_scheduler;
}

void releaseScheduler() {
    HKU_TRACE("releaseScheduler");
    if (g_scheduler) {
        delete g_scheduler;
        g_scheduler = nullptr;
    }
}

}  // namespace hku