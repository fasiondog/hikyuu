/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#include "inner_tasks.h"
#include "scheduler.h"
#include "../../utilities/thread/ThreadPool.h"
#include "../../StockManager.h"

namespace hku {

void InitInnerTask() {
    auto* tm = getScheduler();
    // tm->addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), TimeDelta(), ReloadHikyuuTask);
    tm->addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), TimeDelta(0, 21, 9),
                              ReloadHikyuuTask);
    tm->start();
}

void ReloadHikyuuTask() {
    auto& sm = StockManager::instance();
    ThreadPool tg;
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        tg.submit([=]() {
            Stock stk = *iter;
            auto ktype_list = KQuery::getAllKType();
            for (auto& ktype : ktype_list) {
                if (stk.isBuffer(ktype)) {
                    stk.loadKDataToBuffer(ktype);
                }
            }
        });
    }
}

}  // namespace hku