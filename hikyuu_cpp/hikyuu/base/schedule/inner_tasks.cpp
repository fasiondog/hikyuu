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
}

void ReloadHikyuuTask() {
    HKU_INFO("ReloadHikyuuTask");
    auto& sm = StockManager::instance();
    auto* tg = getGlobalTaskGroup();
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        addTask([=]() {
            Stock stk = *iter;
            auto ktype_list = KQuery::getAllKType();
            for (auto& ktype : ktype_list) {
                if (stk.isBuffer(ktype)) {
                    stk.releaseKDataBuffer(ktype);
                    stk.loadKDataToBuffer(ktype);
                }
            }
        });
    }
}

}  // namespace hku