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
    int64_t hh = 0, mm = 0;
    bool reload_enable = false;
    try {
        auto const& param = StockManager::instance().getHikyuuParameter();
        string reload_time = param.tryGet<string>("reload_time", "00:00");
        auto hh_mm = split(reload_time, ":");
        HKU_CHECK(hh_mm.size() == 2, "reload_time format error: {}", reload_time);

        hh = std::stoll(string(hh_mm[0]));
        mm = std::stoll(string(hh_mm[1]));
        HKU_CHECK(hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59, "reload_time format error: {}",
                  reload_time);
        reload_enable = true;
    } catch (const std::exception& e) {
        HKU_ERROR("Can't auto reload!", e.what());
    }

    if (reload_enable) {
        // shm客户端延迟5分钟重启，以便shm server先重启完毕
        if (StockManager::instance().isIpcClientMode()) {
            constexpr int64_t shm_client_reload_delay_minutes = 5;
            mm += shm_client_reload_delay_minutes;
            hh += mm / 60;
            mm %= 60;
            hh %= 24;  // 24 小时回卷（如 23:59 延后即为次日 00:04，避免超出 TimeDelta 上限）
            HKU_INFO(
              "Running in shm client mode, daily auto reload delayed {} minutes to {:02d}:{:02d}",
              shm_client_reload_delay_minutes, hh, mm);
        }

        auto* tm = getScheduler();
        tm->addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), TimeDelta(0, hh, mm),
                                  reloadHikyuuTask);
    }
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