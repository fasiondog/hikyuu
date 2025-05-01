/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include "hikyuu/StockManager.h"
#include "interface/plugins.h"
#include "device.h"

namespace hku {

void HKU_API activeDevice(const std::string& active_code) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_DEVICE);
    plugin->activate(active_code);
}

std::string HKU_API viewLicense() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    if (!plugin) {
        return fmt::format("Can't find {} plugin!", HKU_PLUGIN_DEVICE);
    }
    return plugin->viewLicense();
}

void HKU_API removeLicense() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_DEVICE);
    plugin->removeLicense();
}

std::string HKU_API fetchTrialLicense(const std::string& email) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    if (!plugin) {
        return fmt::format("Can't find {} plugin!", HKU_PLUGIN_DEVICE);
    }
    return plugin->fetchTrialLicense(email);
}

}  // namespace hku