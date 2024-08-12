/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <csignal>
#include <unordered_set>
#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/ini_parser/IniParser.h"
#include "hikyuu/global/schedule/scheduler.h"
#include "hikyuu/global/GlobalTaskGroup.h"
#include "hikyuu/global/sysinfo.h"
#include "hikyuu/hikyuu.h"
#include "StrategyBase.h"

// python 中运行拉回主线程循环
// c++ 则直接执行（通常在定时调度的工作线程中执行）
#define EVENT(func)          \
    if (runningInPython()) { \
        event(func);         \
    } else {                 \
        func();              \
    }

namespace hku {

std::atomic_bool StrategyBase::ms_keep_running = true;

void StrategyBase::sig_handler(int sig) {
    if (sig == SIGINT) {
        ms_keep_running = false;
        exit(0);
    }
}

StrategyBase::StrategyBase() : StrategyBase("Strategy") {}

StrategyBase::StrategyBase(const string& name) {
    string home = getUserDir();
    HKU_ERROR_IF(home == "", "Failed get user home path!");
#if HKU_OS_WINOWS
    m_config_file = format("{}\\{}", home, ".hikyuu\\hikyuu.ini");
#else
    m_config_file = format("{}/{}", home, ".hikyuu/hikyuu.ini");
#endif
}

StrategyBase::StrategyBase(const string& name, const string& config_file)
: m_name(name), m_config_file(config_file) {}

StrategyBase::~StrategyBase() {
    HKU_INFO("[Strategy {}] Quit Strategy!", m_name);
}

void StrategyBase::run() {
    HKU_IF_RETURN(m_running, void());

    // 调用 strategy 自身的初始化方法
    init();

    StockManager& sm = StockManager::instance();

    // 非独立进程方式运行 Stratege 或 重复执行，则直接返回
    if (sm.thread_id() == std::this_thread::get_id()) {
        return;
    }

    // 注册 ctrl-c 终止信号
    std::signal(SIGINT, sig_handler);

    HKU_INFO("[Strategy {}] strategy is running! You can press Ctrl-C to terminte ...", m_name);

    // 初始化
    hikyuu_init(m_config_file, false, m_context);

    // 先将行情接收代理停止，以便后面加入处理函数
    stopSpotAgent();

    // 根据上下文获取支持的 Stock 列表
    const auto& stk_code_list = getStockCodeList();
    m_stock_list.reserve(stk_code_list.size());
    for (const auto& code : stk_code_list) {
        Stock stk = getStock(code);
        if (!stk.isNull()) {
            m_stock_list.push_back(stk);
        } else {
            HKU_WARN("[Strategy {}] Invalid code: {}, can't find the stock!", m_name, code);
        }
    }
    HKU_WARN_IF(m_stock_list.empty(), "[Strategy {}] stock list is empty!", m_name);

    auto& agent = *getGlobalSpotAgent();
    agent.addProcess(
      [this](const SpotRecord& spot) { EVENT([=]() { this->receivedSpot(spot); }); });
    agent.addPostProcess(
      [this](Datetime revTime) { EVENT([=]() { this->onReceivedSpot(revTime); }); });
    startSpotAgent(false);

    m_running = true;
}

void StrategyBase::start() {
    HKU_INFO("start even loop ...");
    _startEventLoop();
}

void StrategyBase::receivedSpot(const SpotRecord& spot) {
    Stock stk = getStock(format("{}{}", spot.market, spot.code));
    if (!stk.isNull()) {
        EVENT([=]() { this->onChange(stk, spot); });
    }
}

void StrategyBase::runDaily(std::function<void()>&& func, const TimeDelta& delta,
                            const std::string& market) {
    if (!m_running) {
        run();
    }

    auto* scheduler = getScheduler();
    auto new_func = [=]() {
        const auto& sm = StockManager::instance();
        auto today = Datetime::today();
        int day = today.dayOfWeek();
        if (day == 0 || day == 6 || sm.isHoliday(today)) {
            return;
        }

        auto market_info = sm.getMarketInfo(market);
        Datetime open1 = today + market_info.openTime1();
        Datetime close1 = today + market_info.closeTime1();
        Datetime open2 = today + market_info.openTime2();
        Datetime close2 = today + market_info.closeTime2();
        Datetime now = Datetime::now();
        if ((now > open1 && now < close1) || (now > open2 && now < close2)) {
            EVENT(func);
        }
    };

    const auto& sm = StockManager::instance();
    auto market_info = sm.getMarketInfo(market);
    auto today = Datetime::today();
    auto now = Datetime::now();
    TimeDelta now_time = now - today;
    if (now_time >= market_info.closeTime2()) {
        scheduler->addFuncAtTime(today.nextDay() + market_info.openTime1(), [=]() {
            new_func();
            auto* sched = getScheduler();
            sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
        });

    } else if (now_time >= market_info.openTime2()) {
        int64_t ticks = now_time.ticks() - market_info.openTime2().ticks();
        int64_t delta_ticks = delta.ticks();
        if (ticks % delta_ticks == 0) {
            scheduler->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
        } else {
            auto delay = TimeDelta::fromTicks(delta_ticks - (ticks / delta_ticks) * delta_ticks);
            scheduler->addFuncAtTime(now + delay, [=]() {
                new_func();
                auto* sched = getScheduler();
                sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
            });
        }

    } else if (now_time == market_info.closeTime1()) {
        scheduler->addFuncAtTime(today + market_info.openTime2(), [=]() {
            auto* sched = getScheduler();
            sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
        });

    } else if (now_time < market_info.closeTime1() && now_time >= market_info.openTime1()) {
        int64_t ticks = now_time.ticks() - market_info.openTime1().ticks();
        int64_t delta_ticks = delta.ticks();
        if (ticks % delta_ticks == 0) {
            scheduler->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
        } else {
            auto delay = TimeDelta::fromTicks(delta_ticks - (ticks / delta_ticks) * delta_ticks);
            scheduler->addFuncAtTime(now + delay, [=]() {
                new_func();
                auto* sched = getScheduler();
                sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
            });
        }

    } else if (now_time < market_info.openTime1()) {
        scheduler->addFuncAtTime(today + market_info.openTime1(), [=]() {
            auto* sched = getScheduler();
            sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
        });

    } else {
        HKU_ERROR("Unknown process!");
    }
}

void StrategyBase::runDailyAt(std::function<void()>&& func, const TimeDelta& delta,
                              bool ignoreHoliday) {
    if (!m_running) {
        run();
    }

    auto new_func = [=]() {
        if (!ignoreHoliday) {
            EVENT(func);
            return;
        }

        const auto& sm = StockManager::instance();
        auto today = Datetime::today();
        int day = today.dayOfWeek();
        if (day != 0 && day != 6 && !sm.isHoliday(today)) {
            EVENT(func);
        }
    };

    auto* scheduler = getScheduler();
    scheduler->addFuncAtTimeEveryDay(delta, new_func);
}

/*
 * 在主线程中处理事件队列，避免 python GIL
 */
void StrategyBase::_startEventLoop() {
    while (ms_keep_running) {
        event_type task;
        m_event_queue.wait_and_pop(task);
        if (task.isNullTask()) {
            ms_keep_running = false;
        } else {
            task();
        }
    }
}

}  // namespace hku