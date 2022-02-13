/*
 * IStd.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "IStdev.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IStdev)
#endif

namespace hku {

IStdev::IStdev() : IndicatorImp("STDEV", 1) {
    setParam<int>("n", 10);
}

IStdev::~IStdev() {}

bool IStdev::check() {
    return getParam<int>("n") >= 2;
}

void IStdev::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    Indicator ma = MA(data, n);
    size_t N = n - 1;
    for (size_t i = discard(); i < total; ++i) {
        price_t mean = ma[i];
        price_t sum = 0.0;
        for (size_t j = i + 1 - n; j <= i; ++j) {
            sum += std::pow(data[j] - mean, 2);
        }
        _set(std::sqrt(sum / N), i);
    }
}

void IStdev::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step > 0 && curPos < ind.discard() + step - 1, void());
    size_t start = _get_step_start(curPos, step, ind.discard());
    price_t sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += ind[i];
    }
    price_t mean = sum / step;
    sum = 0.0;
    size_t N = curPos - start;
    for (size_t i = start; i <= curPos; i++) {
        sum += std::pow(ind[i] - mean, 2);
    }
    _set(std::sqrt(sum / N), curPos);
}

Indicator HKU_API STDEV(int n) {
    IndicatorImpPtr p = make_shared<IStdev>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API STDEV(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IStdev>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
