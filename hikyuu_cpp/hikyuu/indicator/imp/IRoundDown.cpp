/*
 * IRoundDown.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */

#include "IRoundDown.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRoundDown)
#endif

namespace hku {

IRoundDown::IRoundDown() : IndicatorImp("ROUNDDOWN", 1) {
    setParam<int>("ndigits", 2);
}

IRoundDown::~IRoundDown() {}

bool IRoundDown::check() {
    return getParam<int>("ndigits") >= 0;
}

void IRoundDown::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("ndigits");
    for (size_t i = m_discard; i < total; ++i) {
        _set(roundDown(data[i], n), i);
    }
}

Indicator HKU_API ROUNDDOWN(int ndigits) {
    IndicatorImpPtr p = make_shared<IRoundDown>();
    p->setParam<int>("ndigits", ndigits);
    return Indicator(p);
}

} /* namespace hku */
