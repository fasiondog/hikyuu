/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#include "factor.h"
#include "interface/plugins.h"

namespace hku {

FactorImpPtr HKU_API createFactorImp(const string& name, const Indicator& formula,
                                     const KQuery::KType& ktype, const string& brief,
                                     const string& details, bool need_persist) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin,
                        make_shared<FactorImp>(name, formula, ktype, brief, details, need_persist),
                        htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    return plugin->createFactorImp(name, formula, ktype, brief, details, need_persist);
}

}  // namespace hku