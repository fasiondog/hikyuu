/*
 * ISin.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "ISin.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISin)
#endif

namespace hku {

ISin::ISin() : IndicatorImp("SIN", 1) {}

ISin::~ISin() {}

bool ISin::check() {
    return true;
}

void ISin::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::sin(src[i]);
    }
}

Indicator HKU_API SIN() {
    return Indicator(make_shared<ISin>());
}

} /* namespace hku */
