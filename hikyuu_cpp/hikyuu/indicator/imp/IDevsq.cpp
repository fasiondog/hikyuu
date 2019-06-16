/*
 * IDevsq.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#include "IDevsq.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDevsq)
#endif


namespace hku {

IDevsq::IDevsq(): IndicatorImp("DEVSQ", 1) {
    setParam<int>("n", 10);
}

IDevsq::~IDevsq() {

}

bool IDevsq::check() {
    int n = getParam<int>("n");
    if (n < 2) {
        HKU_ERROR("Invalid param[n] ! (n >= 2) {}", m_params);
        return false;
    }

    return true;
}

void IDevsq::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    Indicator ma = MA(data, n);
    for (size_t i = discard(); i < total; ++i) {
        price_t mean = ma[i];
        price_t sum = 0.0;
        for (size_t j = i + 1 - n; j <= i; ++j) {
            sum += std::pow(data[j] - mean, 2);
        }
        _set(sum, i);
    }
}

Indicator HKU_API DEVSQ(int n) {
    IndicatorImpPtr p = make_shared<IDevsq>();
    p->setParam<int>("n", n);
    return Indicator(p);
}


} /* namespace hku */
