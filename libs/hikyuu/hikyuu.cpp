/*
 * hikyuu.cpp
 *
 *  Created on: 2011-11-13
 *      Author: fasiondog
 */

#include <boost/filesystem.hpp>
#include <hikyuu_utils/iniparser/IniParser.h>
#include "hikyuu.h"

namespace hku {

void hikyuu_init(const string& config_file_name) {
    IniParser config;
    try {
        config.read(config_file_name);

    } catch (std::invalid_argument& e) {
        HKU_FATAL("Reading configure error!\n" << e.what());
        exit(1);
    } catch (std::logic_error& e) {
        HKU_FATAL("Reading configure error!\n" << e.what());
        exit(1);
    } catch(...) {
        HKU_WARN("Reading configure error! Don't know  error!");
        exit(1);
    }

    Parameter baseParam, blockParam, kdataParam, preloadParam, hkuParam;

    hkuParam.set<string>("tmpdir", config.get("hikyuu", "tmpdir", "."));
    hkuParam.set<string>("logger", config.get("hikyuu", "logger", "."));

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

    option = config.getOptionList("preload");
    for (auto iter = option->begin(); iter != option->end(); ++iter) {
        preloadParam.set<bool>(*iter, config.getBool("preload", *iter));
    }

    StockManager& sm = StockManager::instance();
    sm.init(baseParam, blockParam, kdataParam, preloadParam, hkuParam);
}

Stock getStock(const string& querystr) {
    StockManager& sm = StockManager::instance();
    return sm.getStock(querystr);
}

} /* namespace */
