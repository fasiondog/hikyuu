/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-13
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "OneSideSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OneSideSignal)
#endif

namespace hku {

OneSideSignal::OneSideSignal() : SignalBase("SG_OneSide") {
    setParam<bool>("alternate", false);
    setParam<bool>("is_buy", true);  // 买入信号，否则为添加卖出信号
}

OneSideSignal::OneSideSignal(const Indicator& ind, bool is_buy)
: SignalBase("SG_OneSide"), m_ind(ind.clone()) {
    setParam<bool>("alternate", false);
    setParam<bool>("is_buy", is_buy);
}

OneSideSignal::~OneSideSignal() {}

void OneSideSignal::_checkParam(const string& name) const {
    if (name == "alternate") {
        HKU_CHECK(!getParam<bool>(name), "alternate only be false!");
    }
}

SignalPtr OneSideSignal::_clone() {
    auto p = make_shared<OneSideSignal>();
    p->m_ind = m_ind.clone();
    return p;
}

void OneSideSignal::_calculate(const KData& kdata) {
    Indicator ind = m_ind(kdata);
    HKU_IF_RETURN(ind.empty() || ind.size() != kdata.size(), void());

    bool is_buy = getParam<bool>("is_buy");
    const auto* src = ind.data();
    auto const* ks = kdata.data();
    size_t discard = ind.discard();
    size_t total = ind.size();

    for (size_t i = discard; i < total; ++i) {
        if (src[i] > 0.0) {
            if (is_buy) {
                _addBuySignal(ks[i].datetime);
            } else {
                _addSellSignal(ks[i].datetime);
            }
        }
    }
}

SignalPtr HKU_API SG_OneSide(const Indicator& ind, bool is_buy) {
    return make_shared<OneSideSignal>(ind, is_buy);
}

} /* namespace hku */
