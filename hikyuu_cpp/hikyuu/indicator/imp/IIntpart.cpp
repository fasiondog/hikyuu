/*
 * IIntpart.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-18
 *      Author: fasiondog
 */

#include <cmath>
#include "IIntpart.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIntpart)
#endif

namespace hku {

IIntpart::IIntpart() : IndicatorImp("INTPART", 1) {}

IIntpart::~IIntpart() {}

void IIntpart::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IIntpart::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        value_t v = src[i];
        dst[i] = std::isfinite(v) ? std::trunc(v) : Null<value_t>();
    }
}

Indicator HKU_API INTPART() {
    return Indicator(make_shared<IIntpart>());
}

} /* namespace hku */
