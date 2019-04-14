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

IPow::~IPow() {

}

bool IPow::check() {
    return true;
}

void IPow::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    for (size_t i = m_discard; i < total; ++i) {
        _set(std::pow(data[i], n), i);
    }
}


Indicator HKU_API POW(int n) {
    IndicatorImpPtr p = make_shared<IPow>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
