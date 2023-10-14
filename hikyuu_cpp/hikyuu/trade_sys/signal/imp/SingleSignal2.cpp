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

SignalPtr SingleSignal2::_clone() {
    SingleSignal2* p = new SingleSignal2();
    p->m_ind = m_ind;
    return SignalPtr(p);
}

void SingleSignal2::_calculate() {
    int filter_n = getParam<int>("filter_n");
    double filter_p = getParam<double>("filter_p");

    Indicator ind = m_ind(m_kdata);
    Indicator dev = REF(STDEV(DIFF(ind), filter_n), 1);

    size_t start = dev.discard();
    HKU_IF_RETURN(start < 3, void());

    Indicator buy = ind - REF(LLV(ind, filter_n), 1);
    Indicator sell = REF(HHV(ind, filter_n), 1) - ind;
    size_t total = dev.size();
    for (size_t i = start; i < total; ++i) {
        double filter = filter_p * dev[i];
        if (buy[i] > filter) {
            _addBuySignal(m_kdata[i].datetime);
        } else if (sell[i] > filter) {
            _addSellSignal(m_kdata[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Single2(const Indicator& ind, int filter_n, double filter_p) {
    SingleSignal2* p = new SingleSignal2(ind);
    p->setParam<int>("filter_n", filter_n);
    p->setParam<double>("filter_p", filter_p);
    return SignalPtr(p);
}

} /* namespace hku */
