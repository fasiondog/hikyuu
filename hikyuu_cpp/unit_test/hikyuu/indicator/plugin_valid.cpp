/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-16
 *      Author: fasiondog
 */

#include <hikyuu/plugin/interface/plugins.h>
#include <hikyuu/plugin/extind.h>
#include <hikyuu/plugin/device.h>
#include "plugin_valid.h"

using namespace hku;

bool pluginValid() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<ExtendIndicatorsPluginInterface>(HKU_PLUGIN_EXTEND_INDICATOR);
    return plugin && isValidLicense();
}
