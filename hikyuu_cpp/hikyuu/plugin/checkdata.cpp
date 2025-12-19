/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-12-20
 *      Author: fasiondog
 */

#include "hikyuu/StockManager.h"
#include "checkdata.h"

namespace hku {

std::pair<std::string, vector<std::string>> HKU_API checkData(const StockList& stock_list,
                                                              const Datetime& start_date,
                                                              const Datetime& end_date,
                                                              const KQuery::KType& check_ktype) {
    std::pair<std::string, vector<std::string>> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<CheckDataPluginInterface>(HKU_PLUGIN_CHECK_DATA);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_CHECK_DATA);
    return plugin->checkData(stock_list, start_date, end_date, check_ktype);
}

}  // namespace hku