/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#include "interface/plugins.h"
#include "hkuextra.h"

namespace hku {

void HKU_API registerKTypeExtra(const string& ktype, const string& basetype, int32_t minutes,
                                std::function<Datetime(const Datetime&)> getPhaseEnd) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA);
    plugin->registerKTypeExtra(ktype, basetype, minutes, getPhaseEnd);
}

bool HKU_API isExtraKType(const string& ktype) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, false, "Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA);
    return plugin->isExtraKType(ktype);
}

int32_t HKU_API getKTypeExtraMinutes(const string& ktype) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, 0, "Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA);
    return plugin->getKTypeExtraMinutes(ktype);
}

std::vector<string> HKU_API getExtraKTypeList() {
    std::vector<string> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA);
    ret = plugin->getExtraKTypeList();
    return ret;
}

void HKU_API releaseKExtra() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA);
    plugin->releaseKExtra();
}

KRecordList HKU_API getExtraKRecordList(const Stock& stk, const KQuery& query) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, KRecordList(), "Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA);
    return plugin->getExtraKRecordList(stk, query);
}

}  // namespace hku