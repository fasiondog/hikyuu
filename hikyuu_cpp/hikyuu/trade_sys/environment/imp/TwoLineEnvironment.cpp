/*
 * TwoLineEnviroment.cpp
 *
 *  Created on: 2016年5月17日
 *      Author: Administrator
 */

#include "../../../StockManager.h"
#include "../../../indicator/crt/KDATA.h"
#include "TwoLineEnvironment.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TwoLineEnvironment)
#endif

namespace hku {

TwoLineEnvironment::TwoLineEnvironment() : EnvironmentBase("EV_TwoLine") {
    setParam<string>("market", "SH");
}

TwoLineEnvironment::TwoLineEnvironment(const Indicator& fast, const Indicator& slow)
: EnvironmentBase("EV_TwoLine"), m_fast(fast), m_slow(slow) {
    setParam<string>("market", "SH");
}

TwoLineEnvironment::~TwoLineEnvironment() {}

void TwoLineEnvironment::_checkParam(const string& name) const {
    if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    }
}

EnvironmentPtr TwoLineEnvironment::_clone() {
    auto ptr = make_shared<TwoLineEnvironment>();
    ptr->m_fast = m_fast.clone();
    ptr->m_slow = m_slow.clone();
    return ptr;
}

void TwoLineEnvironment::_calculate() {
    string market = getParam<string>("market");
    const StockManager& sm = StockManager::instance();
    MarketInfo market_info = sm.getMarketInfo(market);
    HKU_IF_RETURN(market_info == Null<MarketInfo>(), void());

    Stock stock = sm.getStock(market + market_info.code());
    KData kdata = stock.getKData(m_query);
    Indicator close = CLOSE(kdata);
    Indicator fast = m_fast(close);
    Indicator slow = m_slow(close);

    size_t total = close.size();
    size_t start = fast.discard() > slow.discard() ? fast.discard() : slow.discard();
    auto const* fast_data = fast.data();
    auto const* slow_data = slow.data();
    auto const* ks = kdata.data();
    for (size_t i = start; i < total; i++) {
        if (fast_data[i] > slow_data[i]) {
            _addValid(ks[i].datetime);
        }
    }
}

EVPtr HKU_API EV_TwoLine(const Indicator& fast, const Indicator& slow, const string& market) {
    EVPtr ptr = make_shared<TwoLineEnvironment>(fast, slow);
    ptr->setParam<string>("market", market);
    return ptr;
}

} /* namespace hku */
