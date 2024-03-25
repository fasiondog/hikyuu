/*
 * ISign.cpp
 *
 * Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

#include "ISign.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISign)
#endif

namespace hku {

ISign::ISign() : IndicatorImp("SGN", 1) {}

ISign::~ISign() {}

void ISign::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();
    for (size_t i = m_discard; i < total; i++) {
        if (src[i] > 0) {
            dst[i] = 1.0;
        } else if (src[i] < 0) {
            dst[i] = -1.0;
        } else {
            dst[i] = 0.0;
        }
    }

    return;
}

Indicator HKU_API SGN() {
    return Indicator(make_shared<ISign>());
}

} /* namespace hku */