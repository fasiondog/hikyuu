/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#include "interface/plugins.h"
#include "hkuviews.h"

#if HKU_ENABLE_ARROW

namespace hku {

//-------------------------------
// Views 扩展
//-------------------------------

std::shared_ptr<arrow::Table> getIndicatorsViewParallel(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const KQuery& query, const string& market) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuViewsPluginInterface>(HKU_PLUGIN_HKU_VIEWS);
    HKU_ERROR_IF_RETURN(!plugin, nullptr, "Can't find {} plugin!", HKU_PLUGIN_HKU_VIEWS);
    return plugin->getIndicatorsViewParallel(stks, inds, query, market);
}

std::shared_ptr<arrow::Table> getIndicatorsViewParallel(const StockList& stks,
                                                        const IndicatorList& inds,
                                                        const Datetime& date, size_t cal_len,
                                                        const KQuery::KType& ktype,
                                                        const string& market) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuViewsPluginInterface>(HKU_PLUGIN_HKU_VIEWS);
    HKU_ERROR_IF_RETURN(!plugin, nullptr, "Can't find {} plugin!", HKU_PLUGIN_HKU_VIEWS);
    return plugin->getIndicatorsViewParallel(stks, inds, date, cal_len, ktype, market);
}

std::shared_ptr<arrow::Table> getMarketViewParallel(const StockList& stks, const Datetime& date,
                                                    const string& market) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuViewsPluginInterface>(HKU_PLUGIN_HKU_VIEWS);
    HKU_ERROR_IF_RETURN(!plugin, nullptr, "Can't find {} plugin!", HKU_PLUGIN_HKU_VIEWS);
    return plugin->getMarketViewParallel(stks, date, market);
}

}  // namespace hku

#endif  // HKU_ENABLE_ARROW