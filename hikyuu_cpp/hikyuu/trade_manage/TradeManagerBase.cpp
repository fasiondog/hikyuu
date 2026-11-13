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

TradeManagerBase::TradeManagerBase(const string& name, const TradeCostPtr& costFunc)
: m_name(name), m_costfunc(costFunc), m_broker_last_datetime(Datetime::now()) {
    setParam<int>("precision", 2);  // Calculation precision
    // Real margin accounting: off by default, so the plain cash books stay unchanged
    setParam<bool>("margin_accounting", false);
    setParam<bool>("settle_with_close", true);
    setParam<bool>("force_liquidate", false);
    // The proration base of the daily margin interest
    setParam<int>("annual_days", 365);
    // The fixed rate of each foreign market, see the currency_rates parameter
    setParam<string>("currency_rates", "");
}

shared_ptr<TradeManagerBase> TradeManagerBase::clone() {
    shared_ptr<TradeManagerBase> p = _clone();
    HKU_CHECK(p, "Invalid ptr from _clone!");
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;
    p->m_broker_last_datetime = m_broker_last_datetime;
    // The clone is a deep copy: the models are cloned instead of being shared, so the copy
    // never drives the model instance of the source
    p->m_costfunc = m_costfunc ? m_costfunc->clone() : m_costfunc;
    p->m_margin = m_margin ? m_margin->clone() : m_margin;
    // The brokers are live connections, they stay shared by design
    p->m_broker_list = m_broker_list;
    return p;
}

PriceList TradeManagerBase::getFundsCurve(const DatetimeList& dates, const KQuery::KType& ktype) {
    FundsList funds_list = getFundsList(dates, ktype);
    PriceList ret(funds_list.size());
    int precision = getParam<int>("precision");
    for (size_t i = 0, total = funds_list.size(); i < total; i++) {
        ret[i] = roundEx(funds_list[i].total_assets(), precision);
    }
    return ret;
}

PriceList TradeManagerBase::getProfitCurve(const DatetimeList& dates, const KQuery::KType& ktype) {
    FundsList funds_list = getFundsList(dates, ktype);
    PriceList ret(funds_list.size());
    int precision = getParam<int>("precision");
    for (size_t i = 0, total = funds_list.size(); i < total; i++) {
        ret[i] = roundEx(funds_list[i].profit(), precision);
    }
    return ret;
}

PriceList TradeManagerBase::getProfitCumChangeCurve(const DatetimeList& dates,
                                                    const KQuery::KType& ktype) {
    FundsList funds_list = getFundsList(dates, ktype);
    PriceList ret(funds_list.size());
    int precision = getParam<int>("precision");
    for (size_t i = 0, total = funds_list.size(); i < total; i++) {
        price_t base = funds_list[i].total_base();
        ret[i] =
          base != 0.0 ? roundEx(funds_list[i].total_assets() / base, precision) : Null<price_t>();
    }
    return ret;
}

PriceList TradeManagerBase::getBaseAssetsCurve(const DatetimeList& dates,
                                               const KQuery::KType& ktype) {
    FundsList funds_list = getFundsList(dates, ktype);
    PriceList ret(funds_list.size());
    for (size_t i = 0, total = funds_list.size(); i < total; i++) {
        ret[i] = funds_list[i].total_base();
    }
    return ret;
}

price_t TradeManagerBase::_currencyRate(const Stock& stock) const {
    // An empty table keeps the single-currency books untouched, without any warning
    HKU_IF_RETURN(stock.isNull() || m_currency_rates.empty(), 1.0);
    if (m_currency_rates.has(stock)) {
        return m_currency_rates.lookup(stock, 1.0);
    }

    // A market missing from a configured table is still billed at 1.0: warn once per market
    {
        // getFundsList values the dates in parallel, so the one-off set needs the lock
        std::lock_guard<std::mutex> lock(m_fx_warned_mutex);
        HKU_IF_RETURN(!m_fx_warned.insert(stock.market()).second, 1.0);
    }
    HKU_WARN("{} {} has no currency rate, its amounts are billed at 1.0!", m_name,
             stock.market_code());
    return 1.0;
}

FundsList TradeManagerBase::getFundsList(const DatetimeList& dates, const KQuery::KType& ktype) {
    size_t total = dates.size();
    FundsList result;
    HKU_IF_RETURN(total == 0, result);

    // Calculate the last date first, to prevent the thread safety issue of updating the
    // ex-rights/ex-dividend data in the subsequent parallel calculation
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

std::vector<PositionExtInfo> TradeManagerBase::getHistoryPositionExtInfoList(
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

std::unordered_map<Stock, PositionExtInfo> TradeManagerBase::getPositionExtInfoDict(
  const Datetime& datetime, const KQuery::KType& ktype, int trade_mode) {
    auto ext_info_list = getPositionExtInfoList(datetime, ktype, trade_mode);
    std::unordered_map<Stock, PositionExtInfo> ret;
    for (auto& info : ext_info_list) {
        ret.insert(std::make_pair(info.position.stock, info));
    }
    return ret;
}

PositionExtInfo TradeManagerBase::getPositionExtInfo(const Stock& stock,
                                                     const Datetime& current_time,
                                                     const KQuery::KType& ktype, int trade_mode) {
    PositionExtInfo ret;
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    HKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HKU_PLUGIN_TMREPORT);
    ret = plugin->getPositionExtInfo(shared_from_this(), stock, current_time, ktype, trade_mode);
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

price_t TradeManagerBase::getOpenMarginToLock(const Datetime& datetime, const Stock& stock,
                                              price_t price, double num, MarginSide side) const {
    price_t raw = getOpenMargin(datetime, stock, price, num, side);
    HKU_IF_RETURN(!(raw > 0.0), 0.0);

    int precision = getParam<int>("precision");
    price_t locked = roundEx(raw, precision);
    return locked > 0.0 ? locked : 1.0 / std::pow(10.0, double(precision));
}

bool TradeManagerBase::isMarginAccounted(const Stock& stock) const {
    HKU_IF_RETURN(!m_margin || stock.isNull(), false);
    // An archive written before this switch existed carries no such key
    HKU_IF_RETURN(!tryGetParam<bool>("margin_accounting", false), false);
    HKU_IF_RETURN(!m_margin->isMarginTrading(stock), false);
    bool borrow_cash = haveParam("support_borrow_cash") && getParam<bool>("support_borrow_cash");
    return borrow_cash || !m_margin->needBorrowAsset(stock);
}

price_t TradeManagerBase::getOccupiedMargin(const Datetime& datetime, KQuery::KType ktype) const {
    price_t result = 0.0;
    HKU_IF_RETURN(!m_margin, result);

    // Mirror the valuation moment of getFunds: an unset date or the last trade date values at
    // the last trade moment, any other date at the close of that day
    Datetime valued = _valuationDatetime(datetime);

    PositionRecordList positions = getPositionList();
    for (const auto& record : positions) {
        if (!m_margin->isMarginTrading(record.stock))
            continue;
        price_t price = record.stock.getMarketValue(valued, ktype);
        // The model answers in the security's currency, the account sums in its own
        result += toAccount(record.stock, m_margin->getOpenMargin(valued, record.stock, price,
                                                                  record.number, MarginSide::LONG));
    }

    positions = getShortPositionList();
    for (const auto& record : positions) {
        if (!m_margin->isMarginTrading(record.stock))
            continue;
        price_t price = record.stock.getMarketValue(valued, ktype);
        result += toAccount(
          record.stock,
          m_margin->getOpenMargin(valued, record.stock, price, record.number, MarginSide::SHORT));
    }
    return result;
}

price_t TradeManagerBase::getAvailableMargin(const Datetime& datetime, KQuery::KType ktype) {
    HKU_CHECK(datetime == Null<Datetime>() || datetime >= lastDatetime(),
              "getAvailableMargin supports only the current or a future moment!");
    // Read in the documented order: getFunds applies the pending weights and thus changes the
    // holdings the other two operands then price
    price_t net_assets = getFunds(datetime, ktype).net_assets();
    price_t haircut = getCollateralHaircut(datetime, ktype);
    return net_assets - haircut - getOccupiedMargin(datetime, ktype);
}

price_t TradeManagerBase::getCollateralHaircut(const Datetime& datetime,
                                               KQuery::KType ktype) const {
    // The current holdings are what gets discounted, so a past date would pair them with a
    // stale price, the same restriction getAvailableMargin states
    HKU_CHECK(datetime.isNull() || datetime >= lastDatetime(),
              "getCollateralHaircut supports only the current or a future moment!");
    price_t result = 0.0;
    HKU_IF_RETURN(!m_margin, result);

    Datetime valued = _valuationDatetime(datetime);
    int precision = getParam<int>("precision");
    for (const auto& record : getPositionList()) {
        if (!m_margin->isMarginTrading(record.stock))
            continue;
        double rate = getCollateralRate(valued, record.stock, MarginSide::LONG);
        // A rate outside (0, 1] breaks the contract of the interface, which a Python model can
        // do silently: discount nothing for it and warn, the way a bad margin rate falls back
        // to the cash caliber
        if (!(rate > 0.0 && rate <= 1.0)) {
            HKU_WARN(
              "{} {} collateral rate({}) is out of (0, 1], the holding is not "
              "discounted!",
              valued, record.stock.market_code(), rate);
            continue;
        }
        // A rate of 1.0 (the default) discounts nothing, so it needs no pricing at all
        if (rate >= 1.0) {
            continue;
        }
        // The nominal is rounded before being discounted, exactly like the margin bases, so the
        // result does not drift by half a cent per holding
        price_t mv =
          roundEx(record.stock.getMarketValue(valued, ktype) * record.number * record.stock.unit(),
                  precision);
        // The discount is a share of the net assets, so the base is measured in the account
        // currency
        price_t base = toAccount(record.stock, mv);
        if (record.margin > 0.0) {
            // Only the equity contribution of a margin-accounted holding counts as collateral,
            // and a loss deeper than the margin leaves nothing of it to discount; the locked
            // margin is already booked in the account currency, the floating profit is not
            base =
              roundEx(record.margin + toAccount(record.stock, mv - record.settleBasis), precision);
            if (base < 0.0) {
                base = 0.0;
            }
        }
        result = roundEx(result + base * (1.0 - rate), precision);
    }
    return result;
}

}  // namespace hku