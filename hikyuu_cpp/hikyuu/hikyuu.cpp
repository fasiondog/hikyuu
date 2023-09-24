/*
 * hikyuu.cpp
 *
 *  Created on: 2011-11-13
 *      Author: fasiondog
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <set>
#include <fmt/format.h>
#include "utilities/IniParser.h"
#include "hikyuu.h"
#include "version.h"

namespace hku {

static Parameter g_hikyuu_context;

void hikyuu_init(const string& config_file_name, bool ignore_preload,
                 const StrategyContext& context) {
    IniParser config;
    try {
        config.read(config_file_name);

    } catch (std::invalid_argument& e) {
        HKU_FATAL("Reading configure error! {}", e.what());
        exit(1);
    } catch (std::logic_error& e) {
        HKU_FATAL("Reading configure error! {}", e.what());
        exit(1);
    } catch (...) {
        HKU_WARN("Reading configure error! Don't know  error!");
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
        if (*iter == "convert") {
            kdataParam.set<bool>(*iter, config.getBool("kdata", *iter));
            continue;
        }
        kdataParam.set<string>(*iter, config.get("kdata", *iter));
    }

    option = config.getOptionList("preload");

    for (auto iter = option->begin(); iter != option->end(); ++iter) {
        try {
            preloadParam.set<bool>(*iter,
                                   ignore_preload ? false : config.getBool("preload", *iter));
        } catch (...) {
            if (!ignore_preload) {
                // 获取预加载的最大数量
                try {
                    preloadParam.set<int>(*iter, config.getInt("preload", *iter));
                } catch (...) {
                    HKU_WARN("Invalid option: {}", *iter);
                }
            }
        }
    }

    StockManager& sm = StockManager::instance();
    sm.init(baseParam, blockParam, kdataParam, preloadParam, hkuParam, context);
}

string getVersion() {
    return HKU_VERSION;
}

std::string HKU_API getVersionWithBuild() {
#if defined(_DEBUG) || defined(DEBUG)
    string mode("debug");
#else
    string mode("release");
#endif
#if defined(__arm__)
    return fmt::format("{}_{}_arm_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#elif defined(__aarch64__)
    return fmt::format("{}_{}_aarch64_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#elif defined(__x86_64__) || defined(_WIN64)
    return fmt::format("{}_{}_x64_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#elif defined(__i386__) || defined(_WIN32)
    return fmt::format("{}_{}_i386_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#else
    return fmt::format("{}_{}_unknow_arch_{}", HKU_VERSION, HKU_VERSION_BUILD, mode);
#endif
}

}  // namespace hku
