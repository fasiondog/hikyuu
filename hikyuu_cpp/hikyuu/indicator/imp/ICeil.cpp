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

void ICeil::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void ICeil::_increment_calculate(const Indicator &data, size_t start_pos) {
    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::ceil(src[i]);
    }
}

Indicator HKU_API CEILING() {
    return Indicator(make_shared<ICeil>());
}

} /* namespace hku */
