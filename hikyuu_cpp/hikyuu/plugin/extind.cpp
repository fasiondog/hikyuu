/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#include "hikyuu/StockManager.h"
#include "interface/plugins.h"
#include "extind.h"

namespace hku {

static Indicator getExtIndicator(const string& name, const Parameter& params) {
    Indicator ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<ExtendIndicatorsPluginInterface>(HKU_PLUGIN_EXTEND_INDICATOR);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_EXTEND_INDICATOR);
    return plugin->getIndicator(name, params);
}

static Indicator getExtIndicator(const string& name, const Indicator& ref_ind,
                                 const Parameter& params) {
    Indicator ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<ExtendIndicatorsPluginInterface>(HKU_PLUGIN_EXTEND_INDICATOR);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_EXTEND_INDICATOR);
    return plugin->getIndicator(name, ref_ind, params);
}

Indicator HKU_API WITHKTYPE(const KQuery::KType& ktype, bool fill_null) {
    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    return getExtIndicator("WITHKTYPE", params);
}

Indicator HKU_API WITHKTYPE(const Indicator& ind, const KQuery::KType& ktype, bool fill_null) {
    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    return getExtIndicator("WITHKTYPE", ind, params);
}

}  // namespace hku