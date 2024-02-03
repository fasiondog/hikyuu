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

EnvironmentPtr BoolEnvironment::_clone() {
    return make_shared<BoolEnvironment>(m_ind.clone());
}

void BoolEnvironment::_calculate() {
    string market = getParam<string>("market");
    const StockManager& sm = StockManager::instance();
    MarketInfo market_info = sm.getMarketInfo(market);
    HKU_ERROR_IF_RETURN(market_info == Null<MarketInfo>(), void(), "Can't find maket({}) info!",
                        market);

    Stock stock = sm.getStock(market + market_info.code());
    KData kdata = stock.getKData(m_query);

    auto ds = kdata.getDatetimeList();
    m_ind.setContext(kdata);
    for (size_t i = m_ind.discard(), len = m_ind.size(); i < len; i++) {
        if (!std::isnan(m_ind[i]) && m_ind[i] > 0.) {
            _addValid(ds[i]);
        }
    }
}

EVPtr HKU_API EV_Bool(const Indicator& ind, const string& market) {
    BoolEnvironment* p = new BoolEnvironment(ind);
    p->setParam<string>("market", market);
    return EVPtr(p);
}

}  // namespace hku