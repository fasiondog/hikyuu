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
    StockManager& sm = StockManager::instance();
    sm.init(config_file_name);
}

Stock getStock(const string& querystr) {
    StockManager& sm = StockManager::instance();
    return sm.getStock(querystr);
}

} /* namespace */
