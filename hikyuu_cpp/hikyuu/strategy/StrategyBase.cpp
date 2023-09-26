/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <csignal>
#include <unordered_set>
#include "../utilities/os.h"
#include "../utilities/IniParser.h"
#include "../global/schedule/scheduler.h"
#include "StrategyBase.h"

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
    _initDefaultParam();
}

StrategyBase::StrategyBase(const string& name, const string& config_file)
: m_name(name), m_config_file(config_file) {
    _initDefaultParam();
}

StrategyBase::~StrategyBase() {
    HKU_INFO("[Strategy {}] Quit Strategy!", m_name);
}

void StrategyBase::_initDefaultParam() {
    setParam<bool>("enable_market_event", false);
    setParam<bool>("enable_30_seconds_clock", false);
    setParam<bool>("enable_1min_clock", false);
    setParam<bool>("enable_3min_clock", false);
    setParam<bool>("enable_5min_clock", false);
    setParam<bool>("enable_10min_clock", false);
    setParam<bool>("enable_15min_clock", false);
    setParam<bool>("enable_30min_clock", false);
    setParam<bool>("enable_60min_clock", false);
    setParam<bool>("enable_2hour_clock", false);
}

void StrategyBase::run() {
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

    // 加载上下文指定的证券数据
    IniParser config;
    try {
        config.read(m_config_file);

    } catch (std::exception& e) {
        HKU_FATAL("[Strategy {}] Failed read configure file (\"{}\")! {}", m_name, m_config_file,
                  e.what());
        HKU_INFO("[Strategy {}] Exit Strategy", m_name);
        exit(1);
    } catch (...) {
        HKU_FATAL("[Strategy {}] Failed read configure file (\"{}\")! Unknow error!", m_name,
                  m_config_file);
        HKU_INFO("[Strategy {}] Exit Strategy", m_name);
        exit(1);
    }

    Parameter baseParam, blockParam, kdataParam, preloadParam, hkuParam;

    hkuParam.set<string>("tmpdir", config.get("hikyuu", "tmpdir", "."));
    hkuParam.set<string>("datadir", config.get("hikyuu", "datadir", "."));

    if (!config.hasSection("baseinfo")) {
        HKU_FATAL("Missing configure of baseinfo!");
        exit(1);
    }

    IniParser::StringListPtr option = config.getOptionList("baseinfo");
    for (auto iter = option->begin(); iter != option->end(); ++iter) {
        string value = config.get("baseinfo", *iter);
        baseParam.set<string>(*iter, value);
    }

    IniParser::StringListPtr block_config = config.getOptionList("block");
    for (auto iter = block_config->begin(); iter != block_config->end(); ++iter) {
        string value = config.get("block", *iter);
        blockParam.set<string>(*iter, value);
    }

    option = config.getOptionList("kdata");
    for (auto iter = option->begin(); iter != option->end(); ++iter) {
        kdataParam.set<string>(*iter, config.get("kdata", *iter));
    }

    // 设置预加载参数，只加载指定的 ktype 至内存
    auto ktype_list = m_context.getKTypeList();
    if (ktype_list.empty()) {
        // 如果为空，则默认加载日线数据
        ktype_list.push_back(KQuery::DAY);
    }

    for (auto ktype : ktype_list) {
        to_lower(ktype);
        preloadParam.set<bool>(ktype, true);
        string key(format("{}_max", ktype));
        try {
            preloadParam.set<int>(key, config.getInt("preload", key));
        } catch (...) {
            preloadParam.set<int>(key, 4096);
        }
    }

    sm.init(baseParam, blockParam, kdataParam, preloadParam, hkuParam, m_context);

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

    if (m_stock_list.size() > 0) {
        Stock& ref_stk = m_stock_list[0];
        for (auto& ktype : ktype_list) {
            // 由于异步初始化，此处不用通过先getCount再getKRecord的方式获取最后的KRecord
            KRecordList klist = ref_stk.getKRecordList(KQueryByIndex(0, Null<int64_t>(), ktype));
            size_t count = klist.size();
            if (count > 0) {
                m_ref_last_time[ktype] = klist[count - 1].datetime;
            } else {
                m_ref_last_time[ktype] = Null<Datetime>();
            }
        }
    }

    // 启动行情接收代理
    auto& agent = *getGlobalSpotAgent();
    agent.addProcess([this](const SpotRecord& spot) { this->receivedSpot(spot); });
    agent.addPostProcess([this](Datetime revTime) { this->finishReceivedSpot(revTime); });
    startSpotAgent(false);

    _addTimer();

    _startEventLoop();
}

void StrategyBase::receivedSpot(const SpotRecord& spot) {
    Stock stk = getStock(format("{}{}", spot.market, spot.code));
    if (!stk.isNull()) {
        m_spot_map[stk] = spot;
    }
}

void StrategyBase::finishReceivedSpot(Datetime revTime) {
    HKU_IF_RETURN(m_stock_list.empty(), void());
    event([this]() { this->onTick(); });

    Stock& ref_stk = m_stock_list[0];
    const auto& ktype_list = getKTypeList();
    for (const auto& ktype : ktype_list) {
        size_t count = ref_stk.getCount(ktype);
        if (count > 0) {
            KRecord k = ref_stk.getKRecord(count - 1, ktype);
            if (k.datetime != m_ref_last_time[ktype]) {
                m_ref_last_time[ktype] = k.datetime;
                event([this, ktype]() { this->onBar(ktype); });
            }
        }
    }
}

void StrategyBase::_addTimer() {
    std::unordered_set<string> market_set;
    for (auto& stk : m_stock_list) {
        market_set.insert(stk.market());
    }

    auto& sm = StockManager::instance();
    TimeDelta openTime(0, 23, 59, 59, 999, 999), closeTime(0);
    for (auto& market : market_set) {
        auto market_info = sm.getMarketInfo(market);
        if (market_info.openTime1() < market_info.closeTime1()) {
            if (market_info.openTime1() < openTime) {
                openTime = market_info.openTime1();
            }
            if (market_info.closeTime1() > closeTime) {
                closeTime = market_info.closeTime1();
            }
        }
        if (market_info.openTime2() < market_info.closeTime2()) {
            if (market_info.openTime2() < openTime) {
                openTime = market_info.openTime2();
            }
            if (market_info.closeTime2() > closeTime) {
                closeTime = market_info.closeTime2();
            }
        }
    }

    HKU_ERROR_IF_RETURN(openTime >= closeTime, void(), "Invalid market openTime: {}, closeTime: {}",
                        openTime, closeTime);

    auto* scheduler = getScheduler();
    if (getParam<bool>("enable_market_event")) {
        scheduler->addFuncAtTimeEveryDay(
          openTime, [this]() { this->event([this]() { this->onMarketOpen(); }); });
        scheduler->addFuncAtTimeEveryDay(
          closeTime, [this]() { this->event([this]() { this->onMarketClose(); }); });
    }

    _addClockEvent("enable_30_seconds_clock", Seconds(30), openTime, closeTime);
    _addClockEvent("enable_1min_clock", Minutes(1), openTime, closeTime);
    _addClockEvent("enable_3min_clock", Minutes(3), openTime, closeTime);
    _addClockEvent("enable_5min_clock", Minutes(5), openTime, closeTime);
    _addClockEvent("enable_10min_clock", Minutes(10), openTime, closeTime);
    _addClockEvent("enable_15min_clock", Minutes(15), openTime, closeTime);
    _addClockEvent("enable_30min_clock", Minutes(30), openTime, closeTime);
    _addClockEvent("enable_60min_clock", Minutes(60), openTime, closeTime);
    _addClockEvent("enable_2hour_clock", Hours(2), openTime, closeTime);
}

void StrategyBase::_addClockEvent(const string& enable, TimeDelta delta, TimeDelta openTime,
                                  TimeDelta closeTime) {
    auto* scheduler = getScheduler();
    if (getParam<bool>(enable)) {
        int repeat = static_cast<int>((closeTime - openTime) / delta);
        scheduler->addFunc(Datetime::min(), Datetime::max(), openTime, closeTime, repeat, delta,
                           [this, delta]() { [this, delta]() { this->onClock(delta); }; });
    }
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