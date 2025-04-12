/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include "hikyuu/StockManager.h"
#include "hikyuu/plugin/plugins.h"
#include "device.h"

namespace hku {

void HKU_API activateDevice(const std::string& active_code) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_DEVICE);
    plugin->activate(active_code);
}

}  // namespace hku