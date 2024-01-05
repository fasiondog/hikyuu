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

void initInnerTask() {
    auto* tm = getScheduler();
    tm->addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), TimeDelta(), reloadHikyuuTask);
    tm->start();
}

void reloadHikyuuTask() {
    StockManager::instance().reload();
}

}  // namespace hku