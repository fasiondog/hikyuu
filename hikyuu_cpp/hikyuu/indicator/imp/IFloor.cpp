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

bool IFloor::check() {
    return true;
}

void IFloor::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    for (size_t i = m_discard; i < total; ++i) {
        _set(std::floor(data[i]), i);
    }
}

Indicator HKU_API FLOOR() {
    return Indicator(make_shared<IFloor>());
}

} /* namespace hku */
