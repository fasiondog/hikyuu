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
#include "utilities/ini_parser/IniParser.h"
#include "hikyuu.h"
#include "version.h"

namespace hku {

static Parameter g_hikyuu_context;

void hikyuu_init(const string& config_file_name, bool ignore_preload,
                 const StrategyContext& context) {
    Parameter baseParam, blockParam, kdataParam, preloadParam, hkuParam;
    getConfigFromIni(config_file_name, baseParam, blockParam, kdataParam, preloadParam, hkuParam);
    if (ignore_preload) {
        const auto& ktypes = KQuery::getAllKType();
        for (const auto& ktype : ktypes) {
            string low_ktype = ktype;
            to_lower(low_ktype);
            preloadParam.set<bool>(low_ktype, false);
        }
    }

    StockManager& sm = StockManager::instance();
    sm.init(baseParam, blockParam, kdataParam, preloadParam, hkuParam, context);
}

void HKU_API getConfigFromIni(const string& config_file_name, Parameter& baseParam,
                              Parameter& blockParam, Parameter& kdataParam, Parameter& preloadParam,
                              Parameter& hkuParam) {
    IniParser config;
    config.read(config_file_name);

    hkuParam.set<string>("tmpdir", config.get("hikyuu", "tmpdir", "."));
    hkuParam.set<string>("datadir", config.get("hikyuu", "datadir", "."));
    hkuParam.set<string>("quotation_server",
                         config.get("hikyuu", "quotation_server", "ipc:///tmp/hikyuu_real.ipc"));
    // 加载权息数据
    hkuParam.set<bool>("load_stock_weight", config.getBool("hikyuu", "load_stock_weight", "True"));

    // 加载历史财务数据
    hkuParam.set<bool>("load_history_finance",
                       config.getBool("hikyuu", "load_history_finance", "True"));

    // 插件目录
    hkuParam.set<string>("plugindir",
                         config.get("hikyuu", "plugindir", fmt::format("{}/plugin", getUserDir())));

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

    const auto& ktypes = KQuery::getAllKType();
    for (const auto& ktype : ktypes) {
        string low_ktype = ktype;
        to_lower(low_ktype);
        preloadParam.set<bool>(low_ktype, config.getBool("preload", low_ktype, "False"));
        string num_preload = fmt::format("{}_max", low_ktype);
        preloadParam.set<int>(num_preload, config.getInt("preload", num_preload, "4096"));
    }
}

}  // namespace hku
