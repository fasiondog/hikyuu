/*
 * IPow.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "IPow.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IPow)
#endif

namespace hku {

IPow::IPow() : IndicatorImp("POW", 1) {
    setParam<int>("n", 3);
}

IPow::~IPow() {}

void IPow::_checkParam(const string& name) const {}

void IPow::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::pow(src[i], n);
    }
}

void IPow::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    _set(std::pow(ind[curPos], step), curPos);
}

Indicator HKU_API POW(int n) {
    IndicatorImpPtr p = make_shared<IPow>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API POW(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IPow>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
