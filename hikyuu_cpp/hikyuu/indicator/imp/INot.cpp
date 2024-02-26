/*
 * INot.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "INot.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::INot)
#endif

namespace hku {

INot::INot() : IndicatorImp("NOT", 1) {}

INot::~INot() {}

bool INot::check() {
    return true;
}

void INot::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = (src[i] <= 0.0) ? 1.0 : 0.0;
    }
}

Indicator HKU_API NOT() {
    return Indicator(make_shared<INot>());
}

} /* namespace hku */
