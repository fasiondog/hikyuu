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

void IRoundDown::_checkParam(const string& name) const {
    if ("ndigits" == name) {
        // ndigits may be negative (round to the left of the decimal point); bound its magnitude so
        // that 10^|ndigits| stays representable (10^308 < DBL_MAX < 10^309)
        int n = getParam<int>("ndigits");
        HKU_CHECK(n >= -308 && n <= 308, "ndigits ({}) must be in [-308, 308]", n);
    }
}

void IRoundDown::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IRoundDown::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("ndigits");
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, total = data.size(); i < total; ++i) {
        dst[i] = roundDown(src[i], n);
    }
}

Indicator HKU_API ROUNDDOWN(int ndigits) {
    IndicatorImpPtr p = make_shared<IRoundDown>();
    p->setParam<int>("ndigits", ndigits);
    return Indicator(p);
}

} /* namespace hku */
