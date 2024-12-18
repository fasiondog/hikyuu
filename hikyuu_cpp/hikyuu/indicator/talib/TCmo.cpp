/*
 * TCmo.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "TCmo.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TCmo)
#endif

namespace hku {

TCmo::TCmo() : IndicatorImp("CMO", 1) {}

TCmo::~TCmo() {}

void TCmo::_calculate(const Indicator &data) {
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

Indicator HKU_API CMO() {
    return Indicator(make_shared<TCmo>());
}

} /* namespace hku */
