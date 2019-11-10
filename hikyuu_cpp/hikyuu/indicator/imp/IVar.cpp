/*
 * IVar.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "IVar.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IVar)
#endif

namespace hku {

IVar::IVar() : IndicatorImp("VAR", 1) {
    setParam<int>("n", 10);
}

IVar::~IVar() {}

bool IVar::check() {
    int n = getParam<int>("n");
    if (n < 2) {
        HKU_ERROR("Invalid param[n] ! (n >= 2) {}", m_params);
        return false;
    }

    return true;
}

void IVar::_calculate(const Indicator& data) {
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
        _set(sum / N, i);
    }
}

Indicator HKU_API VAR(int n) {
    IndicatorImpPtr p = make_shared<IVar>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
