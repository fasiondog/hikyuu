/*
 * BoolSignal.cpp
 *
 *  Created on: 2017年7月2日
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "BoolSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::BoolSignal)
#endif

namespace hku {

BoolSignal::BoolSignal() : SignalBase("SG_Bool") {}

BoolSignal::BoolSignal(const Indicator& buy, const Indicator& sell)
: SignalBase("SG_Bool"), m_bool_buy(buy), m_bool_sell(sell) {}

BoolSignal::~BoolSignal() {}

SignalPtr BoolSignal::_clone() {
    auto p = make_shared<BoolSignal>();
    p->m_bool_buy = m_bool_buy.clone();
    p->m_bool_sell = m_bool_sell.clone();
    return p;
}

void BoolSignal::_calculate(const KData& kdata) {
    Indicator buy = ALIGN(m_bool_buy(kdata), kdata);
    Indicator sell = ALIGN(m_bool_sell(kdata), kdata);
    HKU_ERROR_IF_RETURN(buy.size() != sell.size(), void(), "buy.size() != sell.size()");

    size_t discard = buy.discard() > sell.discard() ? buy.discard() : sell.discard();
    size_t total = buy.size();
    auto const* buydata = buy.data();
    auto const* selldata = sell.data();
    auto const* ks = kdata.data();
    for (size_t i = discard; i < total; ++i) {
        if (buydata[i] > 0.0)
            _addBuySignal(ks[i].datetime);
        if (selldata[i] > 0.0)
            _addSellSignal(ks[i].datetime);
    }
}

SignalPtr HKU_API SG_Bool(const Indicator& buy, const Indicator& sell, bool alternate) {
    auto p = make_shared<BoolSignal>(buy, sell);
    p->setParam<bool>("alternate", alternate);
    return p;
}

} /* namespace hku */
