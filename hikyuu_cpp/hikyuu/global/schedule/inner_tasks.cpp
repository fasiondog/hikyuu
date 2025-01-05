/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#include "hikyuu/StockManager.h"
#include "hikyuu/global/GlobalSpotAgent.h"
#include "inner_tasks.h"
#include "scheduler.h"

namespace hku {

void initInnerTask() {
    auto* tm = getScheduler();
    tm->addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), TimeDelta(), reloadHikyuuTask);
}

void reloadHikyuuTask() {
    // 先停止行情接收
    auto* agent = getGlobalSpotAgent();
    bool agent_running = agent->isRunning();
    if (agent_running) {
        agent->stop();
    }

    // 重新加载数据
    StockManager::instance().reload();

    // 重新启动行情接收
    if (agent_running) {
        bool print = agent->getPrintFlag();
        size_t work_num = agent->getWorkerNum();
        const string& addr = agent->getServerAddr();
        startSpotAgent(print, work_num, addr);
    }
}

}  // namespace hku