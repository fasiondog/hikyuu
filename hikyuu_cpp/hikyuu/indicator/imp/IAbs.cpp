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

void IAbs::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IAbs::_increment_calculate(const Indicator &data, size_t start_pos) {
    auto const *src = data.data() + start_pos;
    auto *dst = this->data() + start_pos;
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::abs(src[i]);
    }
}

Indicator HKU_API ABS() {
    return Indicator(make_shared<IAbs>());
}

} /* namespace hku */
