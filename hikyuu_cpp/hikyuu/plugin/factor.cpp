/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#include "factor.h"
#include "interface/plugins.h"

namespace hku {

Factor HKU_API getFactor(const string& name, const KQuery::KType& ktype) {
    Factor ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    ret = plugin->getFactor(name, ktype);
    return ret;
}

void HKU_API saveFactor(const Factor& factor) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, void(),
                        htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    plugin->saveFactor(factor);
}

void HKU_API saveSpecialFactorValues(const Factor& factor, const Stock& stock,
                                     const DatetimeList& dates, const PriceList& values,
                                     bool replace) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, void(),
                        htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    plugin->saveSpecialFactorValues(factor, stock, dates, values, replace);
}

void HKU_API removeFactor(const string& name, const KQuery::KType& ktype) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, void(),
                        htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    plugin->removeFactor(name, ktype);
}

FactorList HKU_API getAllFactors() {
    FactorList ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    ret = plugin->getAllFactors();
    return ret;
}

FactorSetList HKU_API getAllFactorSets() {
    FactorSetList ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    ret = plugin->getAllFactorSets();
    return ret;
}

void HKU_API updateAllFactorsValues(const KQuery::KType& ktype) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, void(),
                        htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    plugin->updateAllFactorsValues(ktype);
}

void HKU_API saveFactorSet(const FactorSet& set) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, void(),
                        htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    plugin->saveFactorSet(set);
}

void HKU_API removeFactorSet(const string& name, const KQuery::KType& ktype) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, void(),
                        htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    plugin->removeFactorSet(name, ktype);
}

FactorSet HKU_API getFactorSet(const string& name, const KQuery::KType& ktype) {
    FactorSet ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    ret = plugin->getFactorSet(name, ktype);
    return ret;
}

IndicatorList getValues(const Factor& factor, const StockList& stocks, const KQuery& query,
                        bool align, bool fill_null, bool tovalue, const DatetimeList& align_dates) {
    IndicatorList ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    ret = plugin->getValues(factor, stocks, query, align, fill_null, tovalue, align_dates);
    return ret;
}

vector<IndicatorList> getValues(const FactorSet& factor_set, const StockList& stocks,
                                const KQuery& query, bool align, bool fill_null, bool tovalue,
                                const DatetimeList& align_dates) {
    vector<IndicatorList> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
    HKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HKU_PLUGIN_CLICKHOUSE_DRIVER));
    ret = plugin->getValues(factor_set, stocks, query, align, fill_null, tovalue, align_dates);
    return ret;
}

}  // namespace hku