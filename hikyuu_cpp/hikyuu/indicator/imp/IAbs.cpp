/*
 * IAbs.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "IAbs.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAbs)
#endif

namespace hku {

IAbs::IAbs() : IndicatorImp("ABS", 1) {}

IAbs::~IAbs() {}

bool IAbs::check() {
    return true;
}

void IAbs::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::abs(src[i]);
    }
}

Indicator HKU_API ABS() {
    return Indicator(make_shared<IAbs>());
}

} /* namespace hku */
