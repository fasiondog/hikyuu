/*
 * ISqrt.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */

#include "ISqrt.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISqrt)
#endif

namespace hku {

ISqrt::ISqrt() : IndicatorImp("SQRT", 1) {}

ISqrt::~ISqrt() {}

void ISqrt::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::sqrt(src[i]);
    }
}

Indicator HKU_API SQRT() {
    return Indicator(make_shared<ISqrt>());
}

} /* namespace hku */
