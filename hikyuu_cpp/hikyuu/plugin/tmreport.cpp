/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-27
 *      Author: fasiondog
 */

#include "interface/plugins.h"
#include "tmreport.h"

namespace hku {

price_t HKU_API getMaxPullBack(const TMPtr& tm, const Datetime& date, const KQuery::KType& ktype) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, 0.0, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    return plugin->getMaxPullBack(tm, date, ktype);
}

std::vector<PositionExtInfo> HKU_API getPositionExtInfoList(const TMPtr& tm,
                                                            const Datetime& current_time,
                                                            const KQuery::KType& ktype,
                                                            int trade_mode) {
    std::vector<PositionExtInfo> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    return plugin->getPositionExtInfoList(tm, current_time, ktype, trade_mode);
}

std::vector<PositionExtInfo> HKU_API getHistoryPositionExtInfoList(const TMPtr& tm,
                                                                   const KQuery::KType& ktype,
                                                                   int trade_mode) {
    std::vector<PositionExtInfo> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    return plugin->getHistoryPositionExtInfoList(tm, ktype, trade_mode);
}

Performance HKU_API getExtPerformance(const TMPtr& tm, const Datetime& datetime,
                                      const KQuery::KType& ktype) {
    Performance ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    return plugin->getExtPerformance(tm, datetime, ktype);
}

}  // namespace hku