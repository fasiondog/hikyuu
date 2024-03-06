/*
 * ICeil.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */

#include "ICeil.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICeil)
#endif

namespace hku {

ICeil::ICeil() : IndicatorImp("CEILING", 1) {}

ICeil::~ICeil() {}

bool ICeil::check() {
    return true;
}

void ICeil::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::ceil(src[i]);
    }
}

Indicator HKU_API CEILING() {
    return Indicator(make_shared<ICeil>());
}

} /* namespace hku */
