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
#include "hikyuu/global/GlobalSpotAgent.h"
#include "hikyuu/global/schedule/scheduler.h"
#include "hikyuu/global/GlobalTaskGroup.h"
#include "hikyuu/global/sysinfo.h"
#include "hikyuu/hikyuu.h"
#include "Strategy.h"

// python 中运行拉回主线程循环
// c++ 则直接执行（通常在定时调度的工作线程中执行）
#define EVENT(func)          \
    if (runningInPython()) { \
        event(func);         \
    } else {                 \
        func();              \
    }

namespace hku {

std::atomic_bool Strategy::ms_keep_running = true;

void Strategy::sig_handler(int sig) {
    if (sig == SIGINT) {
        ms_keep_running = false;
        exit(0);
    }
}

Strategy::Strategy() : Strategy("Strategy", "") {}

Strategy::Strategy(const string& name, const string& config_file)
: m_name(name), m_config_file(config_file) {
    if (m_config_file.empty()) {
        string home = getUserDir();
        HKU_ERROR_IF(home == "", "Failed get user home path!");
#if HKU_OS_WINOWS
        m_config_file = format("{}\\{}", home, ".hikyuu\\hikyuu.ini");
#else
        m_config_file = format("{}/{}", home, ".hikyuu/hikyuu.ini");
#endif
    }
}

Strategy::Strategy(const vector<string>& codeList, const vector<KQuery::KType>& ktypeList,
                   const string& name, const string& config_file)
: Strategy(name, config_file) {
    m_context.setStockCodeList(codeList);
    m_context.setKTypeList(ktypeList);
}

Strategy::~Strategy() {
    ms_keep_running = false;
    CLS_INFO("Quit Strategy {}!", m_name);
}

void Strategy::run() {
    CLS_IF_RETURN(m_running, void());

    CLS_CHECK(!getStockCodeList().empty(), "The context does not contain any stocks!");
    CLS_CHECK(!getKTypeList().empty(), "The K type list was empty!");

    StockManager& sm = StockManager::instance();

    // 非独立进程方式运行 Stratege 或 重复执行，则直接返回
    if (sm.thread_id() == std::this_thread::get_id()) {
        return;
    }

    // 注册 ctrl-c 终止信号
    std::signal(SIGINT, sig_handler);

    CLS_INFO("{} is running! You can press Ctrl-C to terminte ...", m_name);

    // 初始化
    hikyuu_init(m_config_file, false, m_context);

    // 先将行情接收代理停止，以便后面加入处理函数
    stopSpotAgent();

    auto& agent = *getGlobalSpotAgent();
    agent.addProcess([this](const SpotRecord& spot) { receivedSpot(spot); });
    agent.addPostProcess([this](Datetime revTime) {
        if (m_on_recieved_spot) {
            EVENT([=]() { m_on_recieved_spot(revTime); });
        }
    });
    startSpotAgent(true);

    m_running = true;
}

void Strategy::start() {
    CLS_CHECK(m_running, "No handler functions are registered!");
    CLS_INFO("start even loop ...");
    _startEventLoop();
}

void Strategy::onChange(std::function<void(const Stock&, const SpotRecord& spot)>&& changeFunc) {
    if (!m_running) {
        run();
    }
    m_on_change = changeFunc;
}

void Strategy::onReceivedSpot(std::function<void(const Datetime&)>&& recievedFucn) {
    if (!m_running) {
        run();
    }
    m_on_recieved_spot = recievedFucn;
}

void Strategy::receivedSpot(const SpotRecord& spot) {
    Stock stk = getStock(format("{}{}", spot.market, spot.code));
    if (!stk.isNull()) {
        if (m_on_change) {
            EVENT([=]() { m_on_change(stk, spot); });
        }
    }
}

void Strategy::runDaily(std::function<void()>&& func, const TimeDelta& delta,
                        const std::string& market) {
    if (!m_running) {
        run();
    }

    try {
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
            if ((now >= open1 && now <= close1) || (now >= open2 && now <= close2)) {
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
                auto delay = TimeDelta::fromTicks((ticks / delta_ticks + 1) * delta_ticks - ticks);
                scheduler->addFuncAtTime(now + delay, [=]() {
                    new_func();
                    auto* sched = getScheduler();
                    sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
                });
            }

        } else if (now_time >= market_info.closeTime1()) {
            scheduler->addFuncAtTime(today + market_info.openTime2(), [=]() {
                new_func();
                auto* sched = getScheduler();
                sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
            });

        } else if (now_time < market_info.closeTime1() && now_time >= market_info.openTime1()) {
            int64_t ticks = now_time.ticks() - market_info.openTime1().ticks();
            int64_t delta_ticks = delta.ticks();
            if (ticks % delta_ticks == 0) {
                scheduler->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
            } else {
                auto delay = TimeDelta::fromTicks((ticks / delta_ticks + 1) * delta_ticks - ticks);
                scheduler->addFuncAtTime(now + delay, [=]() {
                    new_func();
                    auto* sched = getScheduler();
                    sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
                });
            }

        } else if (now_time < market_info.openTime1()) {
            scheduler->addFuncAtTime(today + market_info.openTime1(), [=]() {
                new_func();
                auto* sched = getScheduler();
                sched->addDurationFunc(std::numeric_limits<int>::max(), delta, new_func);
            });

        } else {
            CLS_ERROR("Unknown process! now_time: {}", now_time);
        }
    } catch (const std::exception& e) {
        CLS_THROW(e.what());
    }
}

void Strategy::runDailyAt(std::function<void()>&& func, const TimeDelta& delta,
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
void Strategy::_startEventLoop() {
    while (ms_keep_running) {
        event_type task;
        m_event_queue.wait_and_pop(task);
        if (task.isNullTask()) {
            ms_keep_running = false;
        } else {
            try {
                task();
            } catch (const std::exception& e) {
                CLS_ERROR("Failed run task! {}", e.what());
            } catch (...) {
                CLS_ERROR("Failed run task! Unknow error!");
            }
        }
    }
}

}  // namespace hku