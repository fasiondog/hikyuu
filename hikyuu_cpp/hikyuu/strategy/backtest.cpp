/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include "hikyuu/plugin/plugins.h"
#include "backtest.h"

namespace hku {

void backtest(const StrategyContext& context, const std::function<void(Strategy*)>& on_bar,
              const TradeManagerPtr& tm, const Datetime& start_date, const Datetime& end_date,
              const KQuery::KType& ktype, const string& ref_market, int mode) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<BackTestPluginInterface>(HKU_PLUGIN_BACKTEST);
    HKU_ERROR_IF_RETURN(!plugin, void(), "Can't find {} plugin!", HKU_PLUGIN_BACKTEST);
    plugin->backtest(context, on_bar, tm, start_date, end_date, ktype, ref_market, mode);
}

}  // namespace hku