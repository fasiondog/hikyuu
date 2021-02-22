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
    }
}

StrategyBase::StrategyBase() : StrategyBase("Strategy") {}

StrategyBase::StrategyBase(const string& name) {
    string home = getUserHome();
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
    HKU_INFO("Quit Strategy {}", m_name);
}

void StrategyBase::run() {
    HKU_INFO("Strategy {} is running! You can press Ctrl-C to terminte ...", m_name);

    // 注册 ctrl-c 终止信号
    std::signal(SIGINT, sig_handler);

    // 调用 strategy 自身的初始化方法
    init();

    // 加载上下文指定的证券数据
    IniParser config;
    try {
        config.read(m_config_file);

    } catch (std::exception& e) {
        HKU_FATAL("Failed read configure file (\"{}\")! {}", m_config_file, e.what());
        HKU_INFO("Exit Strategy {}", m_name);
        exit(1);
    } catch (...) {
        HKU_FATAL("Failed read configure file (\"{}\")! Unknow error!", m_config_file);
        HKU_INFO("Exit Strategy {}", m_name);
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

    StockManager& sm = StockManager::instance();
    sm.init(baseParam, blockParam, kdataParam, preloadParam, hkuParam, m_context);

    // 启动行情接收代理
    auto& agent = *getGlobalSpotAgent();
    agent.addProcess([this](const SpotRecord& spot) { this->receivedSpot(spot); });
    agent.addPostProcess([this](Datetime revTime) { this->finishReceivedSpot(revTime); });
    startSpotAgent(true);

    _startEventLoop();
}

void StrategyBase::receivedSpot(const SpotRecord& spot) {
    Stock stk = getStock(format("{}{}", spot.market, spot.code));
    if (!stk.isNull()) {
        m_spot_map[stk] = spot;
    }
}

void StrategyBase::finishReceivedSpot(Datetime revTime) {
    event([this]() { this->onTick(); });

    const auto& ktype_list = getKTypeList();
    for (const auto& ktype : ktype_list) {
        event([this, ktype]() { this->onBar(ktype); });
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