/*
 * SingleSignal2.cpp
 *
 *  Created on: 2016年4月16日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/KDATA.h"
#include "../../../indicator/crt/DIFF.h"
#include "../../../indicator/crt/STDEV.h"
#include "../../../indicator/crt/HHV.h"
#include "../../../indicator/crt/LLV.h"
#include "../../../indicator/crt/REF.h"
#include "SingleSignal2.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::SingleSignal2)
#endif

namespace hku {

SingleSignal2::SingleSignal2() : SignalBase("SG_Single2") {
    setParam<int>("filter_n", 10);
    setParam<double>("filter_p", 0.1);
}

SingleSignal2::SingleSignal2(const Indicator& ind) : SignalBase("SG_Single2"), m_ind(ind) {
    setParam<int>("filter_n", 10);
    setParam<double>("filter_p", 0.1);
}

SingleSignal2::~SingleSignal2() {}

void SingleSignal2::_checkParam(const string& name) const {
    if ("filter_n" == name) {
        HKU_ASSERT(getParam<int>("filter_n") >= 3);
    } else if ("filter_p" == name) {
        double filter_p = getParam<double>(name);
        HKU_ASSERT(filter_p > 0.0 && filter_p < 1.0);
    }
}

SignalPtr SingleSignal2::_clone() {
    auto p = make_shared<SingleSignal2>();
    p->m_ind = m_ind.clone();
    return p;
}

void SingleSignal2::_calculate(const KData& kdata) {
    int filter_n = getParam<int>("filter_n");
    double filter_p = getParam<double>("filter_p");

    Indicator ind = m_ind(kdata);
    Indicator dev = REF(STDEV(DIFF(ind), filter_n), 1);

    size_t start = dev.discard();
    HKU_IF_RETURN(start < 3, void());

    Indicator buy = ind - REF(LLV(ind, filter_n), 1);
    Indicator sell = REF(HHV(ind, filter_n), 1) - ind;
    size_t total = dev.size();
    auto const* buydata = buy.data();
    auto const* selldata = sell.data();
    auto const* devdata = dev.data();
    auto const* ks = kdata.data();
    for (size_t i = start; i < total; ++i) {
        double filter = filter_p * devdata[i];
        if (buydata[i] > filter) {
            _addBuySignal(ks[i].datetime);
        } else if (selldata[i] > filter) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Single2(const Indicator& ind, int filter_n, double filter_p) {
    SignalPtr p = make_shared<SingleSignal2>(ind);
    p->setParam<int>("filter_n", filter_n);
    p->setParam<double>("filter_p", filter_p);
    return p;
}

} /* namespace hku */
