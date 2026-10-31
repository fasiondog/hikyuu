/*
 * IFloor.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */

#include "IFloor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IFloor)
#endif

namespace hku {

IFloor::IFloor() : IndicatorImp("FLOOR", 1) {}

IFloor::~IFloor() {}

void IFloor::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IFloor::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::floor(src[i]);
    }
}

Indicator HKU_API FLOOR() {
    return Indicator(make_shared<IFloor>());
}

} /* namespace hku */
