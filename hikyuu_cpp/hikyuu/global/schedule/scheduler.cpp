/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#include "hikyuu/GlobalInitializer.h"
#include "hikyuu/utilities/Log.h"
#include <mutex>
#include "scheduler.h"

namespace hku {

static TimerManager *g_scheduler;

TimerManager *getScheduler() {
    static std::once_flag oc;
    std::call_once(oc, [&]() { g_scheduler = new TimerManager(2); });
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