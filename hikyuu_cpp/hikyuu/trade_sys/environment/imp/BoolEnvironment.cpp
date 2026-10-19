/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-03
 *      Author: fasiondog
 */

#include "hikyuu/StockManager.h"
#include "BoolEnvironment.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::BoolEnvironment)
#endif

namespace hku {

BoolEnvironment::BoolEnvironment() : EnvironmentBase("EV_Bool") {
    setParam<string>("market", "SH");
}

BoolEnvironment::BoolEnvironment(const Indicator& ind) : EnvironmentBase("EV_Bool"), m_ind(ind) {
    setParam<string>("market", "SH");
}

BoolEnvironment::~BoolEnvironment() {}

void BoolEnvironment::_checkParam(const string& name) const {
    if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    }
}

EnvironmentPtr BoolEnvironment::_clone() {
    return make_shared<BoolEnvironment>(m_ind.clone());
}

void BoolEnvironment::_calculate() {
    string market = getParam<string>("market");
    const StockManager& sm = StockManager::instance();
    MarketInfo market_info = sm.getMarketInfo(market);
    HKU_ERROR_IF_RETURN(market_info == Null<MarketInfo>(), void(),
                        "Can't find the market({}) info!", market);

    Stock stock = sm.getStock(market + market_info.code());
    KData kdata = stock.getKData(m_query);
    HKU_IF_RETURN(kdata.empty(), void());

    m_ind.setContext(kdata);
    auto const* ind_data = m_ind.data();
    size_t discard = m_ind.discard();
    size_t total = m_ind.size();
    if (total == kdata.size()) {
        const auto* kdata_ptr = kdata.data();
        for (size_t i = discard; i < total; i++) {
            if (std::isfinite(ind_data[i]) && ind_data[i] > 0.) {
                _addValid(kdata_ptr[i].datetime);
            }
        }
    } else {
        DatetimeList dates = m_ind.getDatetimeList();
        for (size_t i = discard; i < total; i++) {
            if (std::isfinite(ind_data[i]) && ind_data[i] > 0.) {
                _addValid(dates[i]);
            }
        }
    }
}

EVPtr HKU_API EV_Bool(const Indicator& ind, const string& market) {
    EVPtr p = make_shared<BoolEnvironment>(ind);
    p->setParam<string>("market", market);
    return p;
}

}  // namespace hku
