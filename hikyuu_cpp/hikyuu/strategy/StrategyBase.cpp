/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include "../utilities/os.h"
#include "../utilities/IniParser.h"
#include "StrategyBase.h"

namespace hku {

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

void StrategyBase::start() {
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

    // 调用 strategy 自身的初始化方法
    init();
}

}  // namespace hku