/*
 * TwoLineEnviroment.cpp
 *
 *  Created on: 2016-5-17
 *      Author: Administrator
 */

#include <cmath>

#include "../../../StockManager.h"
#include "../../../indicator/crt/ALIGN.h"
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
    HKU_ERROR_IF_RETURN(market_info == Null<MarketInfo>(), void(),
                        "Can't find the market({}) info!", market);

    Stock stock = sm.getStock(market + market_info.code());
    KData kdata = stock.getKData(m_query);
    HKU_IF_RETURN(kdata.empty(), void());

    Indicator close = CLOSE(kdata);
    Indicator fast = m_fast(close);
    Indicator slow = m_slow(close);

    size_t total = close.size();
    if (fast.size() != total) {
        fast = ALIGN(fast, kdata);
    }
    if (slow.size() != total) {
        slow = ALIGN(slow, kdata);
    }
    HKU_WARN_IF_RETURN(fast.size() != total || slow.size() != total, void(),
                       "The fast/slow indicator length must match the KData length ({} vs {}/{})!",
                       total, fast.size(), slow.size());

    size_t start = fast.discard() > slow.discard() ? fast.discard() : slow.discard();
    auto const* fast_data = fast.data();
    auto const* slow_data = slow.data();
    auto const* ks = kdata.data();
    for (size_t i = start; i < total; i++) {
        if (std::isfinite(fast_data[i]) && std::isfinite(slow_data[i]) &&
            fast_data[i] > slow_data[i]) {
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
