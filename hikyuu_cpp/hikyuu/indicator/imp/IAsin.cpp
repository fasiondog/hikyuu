/*
 * IAsin.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "IAsin.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAsin)
#endif

namespace hku {

IAsin::IAsin() : IndicatorImp("ASIN", 1) {}

IAsin::~IAsin() {}

bool IAsin::check() {
    return true;
}

void IAsin::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    value_t null_value = Null<value_t>();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = src[i] <= 1.0 && src[i] >= -1.0 ? std::asin(src[i]) : null_value;
    }
}

Indicator HKU_API ASIN() {
    return Indicator(make_shared<IAsin>());
}

} /* namespace hku */
