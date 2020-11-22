/*
 * StdDeviation.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "StdDeviation.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::StdDeviation)
#endif

namespace hku {

StdDeviation::StdDeviation() : IndicatorImp("STD", 1) {
    setParam<int>("n", 10);
}

StdDeviation::~StdDeviation() {}

bool StdDeviation::check() {
    return getParam<int>("n") >= 2;
}

void StdDeviation::_calculate(const Indicator& data) {
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

Indicator HKU_API STDEV(int n) {
    IndicatorImpPtr p = make_shared<StdDeviation>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API STDEV(const Indicator& data, int n) {
    return STDEV(n)(data);
}

} /* namespace hku */
