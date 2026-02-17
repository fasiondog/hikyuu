/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-30
 *      Author: fasiondog
 */

#include "hikyuu/plugin/interface/plugins.h"
#include "hikyuu/plugin/device.h"
#include "TradeManagerBase.h"
#include "Performance.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TradeManagerBase)
#endif

namespace hku {

FundsList TradeManagerBase::getFundsList(const DatetimeList& dates, const KQuery::KType& ktype) {
    size_t total = dates.size();
    FundsList result;
    HKU_IF_RETURN(total == 0, result);

    // 先计算最后日期，防止后续并行计算时更新权息涉及线程安全
    auto last_funds = getFunds(dates.back(), ktype);

    result = global_parallel_for_index(
      0, total - 1, [&, this](size_t i) -> FundsRecord { return getFunds(dates[i], ktype); });

    result.push_back(last_funds);
    return result;
}

Performance TradeManagerBase::getPerformance(const Datetime& datetime, const KQuery::KType& ktype,
                                             bool ext) {
    Performance ret;
    if (ext && isValidLicense()) {
        auto& sm = StockManager::instance();
        auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
        HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
        ret = plugin->getExtPerformance(shared_from_this(), datetime, ktype);
    } else {
        ret = Performance();
        ret.statistics(shared_from_this(), datetime);
    }

    return ret;
}

price_t TradeManagerBase::getMaxPullBack(const Datetime& date, const KQuery::KType& ktype) {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, 0.0, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    return plugin->getMaxPullBack(shared_from_this(), date, ktype);
}

std::vector<PositionExtInfo> TradeManagerBase::TradeManagerBase::getHistoryPositionExtInfoList(
  const KQuery::KType& ktype, int trade_mode) {
    std::vector<PositionExtInfo> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    ret = plugin->getHistoryPositionExtInfoList(shared_from_this(), ktype, trade_mode);
    return ret;
}

std::vector<PositionExtInfo> TradeManagerBase::getPositionExtInfoList(const Datetime& current_time,
                                                                      const KQuery::KType& ktype,
                                                                      int trade_mode) {
    std::vector<PositionExtInfo> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    ret = plugin->getPositionExtInfoList(shared_from_this(), current_time, ktype, trade_mode);
    return ret;
}

std::vector<std::pair<Datetime, double>> TradeManagerBase::getProfitPercentMonthly(
  const Datetime& datetime) {
    std::vector<std::pair<Datetime, double>> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    ret = plugin->getProfitPercentMonthly(shared_from_this(), datetime);
    return ret;
}

std::vector<std::pair<Datetime, double>> TradeManagerBase::getProfitPercentYearly(
  const Datetime& datetime) {
    std::vector<std::pair<Datetime, double>> ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    ret = plugin->getProfitPercentYearly(shared_from_this(), datetime);
    return ret;
}

}  // namespace hku