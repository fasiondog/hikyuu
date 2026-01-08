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

void IAsin::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IAsin::_increment_calculate(const Indicator &data, size_t start_pos) {
    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::asin(src[i]);
    }
}

Indicator HKU_API ASIN() {
    return Indicator(make_shared<IAsin>());
}

} /* namespace hku */
