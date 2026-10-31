/*
 * IAtan.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "IAtan.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAtan)
#endif

namespace hku {

IAtan::IAtan() : IndicatorImp("ATAN", 1) {}

IAtan::~IAtan() {}

void IAtan::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IAtan::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::atan(src[i]);
    }
}

Indicator HKU_API ATAN() {
    return Indicator(make_shared<IAtan>());
}

} /* namespace hku */
