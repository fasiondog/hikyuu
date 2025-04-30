/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-15
 *      Author: fasiondog
 */

#include "interface/plugins.h"
#include "dataserver.h"

namespace hku {

void HKU_API startDataServer(const std::string& addr, size_t work_num) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataServerPluginInterface>(HKU_PLUGIN_DATASERVER);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_DATASERVER);
    plugin->start(addr, work_num);
}

void HKU_API stopDataServer() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataServerPluginInterface>(HKU_PLUGIN_DATASERVER);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_DATASERVER);
    plugin->stop();
}

}  // namespace hku