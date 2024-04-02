/*
 * SingleSignal.cpp
 *
 *  Created on: 2015年2月22日
 *      Author: fasiondog
 */

#include "../../../indicator/crt/KDATA.h"
#include "../../../indicator/crt/DIFF.h"
#include "../../../indicator/crt/STDEV.h"
#include "SingleSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::SingleSignal)
#endif

namespace hku {

SingleSignal::SingleSignal() : SignalBase("SG_Single") {
    setParam<int>("filter_n", 10);
    setParam<double>("filter_p", 0.1);
}

SingleSignal::SingleSignal(const Indicator& ind) : SignalBase("SG_Single"), m_ind(ind) {
    setParam<int>("filter_n", 10);
    setParam<double>("filter_p", 0.1);
}

SingleSignal::~SingleSignal() {}

void SingleSignal::_checkParam(const string& name) const {
    if ("filter_n" == name) {
        HKU_ASSERT(getParam<int>("filter_n") >= 3);
    } else if ("filter_p" == name) {
        double filter_p = getParam<double>(name);
        HKU_ASSERT(filter_p > 0.0 && filter_p < 1.0);
    }
}

SignalPtr SingleSignal::_clone() {
    SingleSignal* p = new SingleSignal();
    p->m_ind = m_ind.clone();
    return SignalPtr(p);
}

void SingleSignal::_calculate(const KData& kdata) {
    int filter_n = getParam<int>("filter_n");
    double filter_p = getParam<double>("filter_p");

    Indicator ind = m_ind(kdata);
    Indicator dev = STDEV(DIFF(ind), filter_n);

    size_t start = dev.discard();
    HKU_IF_RETURN(start < 3, void());

    auto const* inddata = ind.data();
    auto const* devdata = dev.data();
    auto const* ks = kdata.data();
    size_t total = dev.size();
    for (size_t i = start; i < total; ++i) {
        double dama = inddata[i] - inddata[i - 1];
        double dama2 = inddata[i] - inddata[i - 2];
        double dama3 = inddata[i] - inddata[i - 3];
        double sdama = devdata[i] * filter_p;
        if (dama > 0 && (dama > sdama || dama2 > sdama || dama3 > sdama)) {
            _addBuySignal(ks[i].datetime);
        } else if (dama < 0 && (dama < sdama || dama2 < sdama || dama3 < sdama)) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

SignalPtr HKU_API SG_Single(const Indicator& ind, int filter_n, double filter_p) {
    SingleSignal* p = new SingleSignal(ind);
    p->setParam<int>("filter_n", filter_n);
    p->setParam<double>("filter_p", filter_p);
    return SignalPtr(p);
}

} /* namespace hku */
