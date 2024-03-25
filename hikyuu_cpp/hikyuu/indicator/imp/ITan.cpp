/*
 * ITan.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "ITan.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ITan)
#endif

namespace hku {

ITan::ITan() : IndicatorImp("TAN", 1) {}

ITan::~ITan() {}

void ITan::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::tan(src[i]);
    }
}

Indicator HKU_API TAN() {
    return Indicator(make_shared<ITan>());
}

} /* namespace hku */
