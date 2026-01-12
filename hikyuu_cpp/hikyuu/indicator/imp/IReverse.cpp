/*
 * IReverse.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "IReverse.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IReverse)
#endif

namespace hku {

IReverse::IReverse() : IndicatorImp("REVERSE", 1) {}

IReverse::~IReverse() {}

void IReverse::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IReverse::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = -src[i];
    }
}

Indicator HKU_API REVERSE() {
    return Indicator(make_shared<IReverse>());
}

} /* namespace hku */
