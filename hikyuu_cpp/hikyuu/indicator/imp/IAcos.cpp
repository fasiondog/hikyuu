/*
 * IAcos.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "IAcos.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAcos)
#endif

namespace hku {

IAcos::IAcos() : IndicatorImp("ACOS", 1) {}

IAcos::~IAcos() {}

void IAcos::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::acos(src[i]);
    }
}

Indicator HKU_API ACOS() {
    return Indicator(make_shared<IAcos>());
}

} /* namespace hku */
