/*
 * BoolSignal.cpp
 *
 *  Created on: 2017年7月2日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/KDATA.h"
#include "BoolSignal.h"

namespace hku {

BoolSignal::BoolSignal() : SignalBase("SG_Bool") {
    setParam<string>("kpart", "CLOSE");
}

BoolSignal::BoolSignal(const Indicator& buy, const Indicator& sell, const string& kpart)
: SignalBase("SG_Bool"), m_bool_buy(buy), m_bool_sell(sell) {
    setParam<string>("kpart", kpart);
}

BoolSignal::~BoolSignal() {}

SignalPtr BoolSignal::_clone() {
    BoolSignal* p = new BoolSignal();
    p->m_bool_buy = m_bool_buy;
    p->m_bool_sell = m_bool_sell;
    return SignalPtr(p);
}

void BoolSignal::_calculate() {
    string kpart = getParam<string>("kpart");
    Indicator kdata = KDATA_PART(m_kdata, kpart);
    Indicator buy = m_bool_buy(kdata);
    Indicator sell = m_bool_sell(kdata);
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

SignalPtr HKU_API SG_Bool(const Indicator& buy, const Indicator& sell,
                          const string& kpart = "CLOSE") {
    return SignalPtr(new BoolSignal(buy, sell, kpart));
}

} /* namespace hku */
