/*
 * ISma.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

#include "ISma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISma)
#endif

namespace hku {

ISma::ISma() : IndicatorImp("SMA", 1) {
    setParam<int>("n", 22);
    setParam<double>("m", 2.0);
}

ISma::~ISma() {}

bool ISma::check() {
    return getParam<int>("n") >= 1;
}

void ISma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    double n = getParam<int>("n");
    double m = getParam<double>("m");

    double p = n - m;
    _set(ind[m_discard], m_discard);
    for (size_t i = m_discard + 1; i < total; i++) {
        _set((m * ind[i] + p * get(i - 1)) / n, i);
    }
}

Indicator HKU_API SMA(int n, double m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setParam<int>("n", n);
    p->setParam<double>("m", m);
    return Indicator(p);
}

} /* namespace hku */
