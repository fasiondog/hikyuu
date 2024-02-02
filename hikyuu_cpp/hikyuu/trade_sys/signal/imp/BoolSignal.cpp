/*
 * BoolSignal.cpp
 *
 *  Created on: 2017年7月2日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/KDATA.h"
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
    BoolSignal* p = new BoolSignal();
    p->m_bool_buy = m_bool_buy.clone();
    p->m_bool_sell = m_bool_sell.clone();
    return SignalPtr(p);
}

void BoolSignal::_calculate() {
    Indicator buy = m_bool_buy(m_kdata);
    Indicator sell = m_bool_sell(m_kdata);
    HKU_ERROR_IF_RETURN(buy.size() != sell.size(), void(), "buy.size() != sell.size()");

    size_t discard = buy.discard() > sell.discard() ? buy.discard() : sell.discard();
    size_t total = buy.size();
    for (size_t i = discard; i < total; ++i) {
        if (buy[i] > 0.0)
            _addBuySignal(m_kdata[i].datetime);
        if (sell[i] > 0.0)
            _addSellSignal(m_kdata[i].datetime);
    }
}

SignalPtr HKU_API SG_Bool(const Indicator& buy, const Indicator& sell) {
    return SignalPtr(new BoolSignal(buy, sell));
}

} /* namespace hku */
