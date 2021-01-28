/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#include "inner_tasks.h"
#include "scheduler.h"
#include "../GlobalTaskGroup.h"
#include "../../StockManager.h"

namespace hku {

void InitInnerTask() {
    auto* tm = getScheduler();
    tm->addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), TimeDelta(), ReloadHikyuuTask);
    tm->start();
}

void ReloadHikyuuTask() {
    StockManager::instance().reload();
}

}  // namespace hku