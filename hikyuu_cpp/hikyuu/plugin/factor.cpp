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
                                     const string& details, bool need_persist,
                                     const Datetime& start_date, const Block& block) {
    FactorImpPtr ret;
    auto& sm = StockManager::instance();
    const auto& params = sm.getKDataDriverParameter();
    if (params.tryGet<string>("type", "hdf5") != "clickhouse") {
        ret = make_shared<FactorImp>(name, formula, ktype, brief, details, need_persist, start_date,
                                     block);
        return ret;
    }

    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    if (!plugin) {
        ret = make_shared<FactorImp>(name, formula, ktype, brief, details, need_persist, start_date,
                                     block);
        return ret;
    }

    ret = plugin->createFactorImp(name, formula, ktype, brief, details, need_persist, start_date,
                                  block);
    return ret;
}

FactorList HKU_API getAllFactors() {
    FactorList ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    ret = plugin->getAllFactors();
    return ret;
}

void HKU_API updateAllFactorsValues(const KQuery::KType& ktype) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, void(),
                        htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    plugin->updateAllFactorsValues(ktype);
}

}  // namespace hku