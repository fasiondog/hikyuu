/*
 * IBarsLast.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "IBarsLast.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBarsLast)
#endif

namespace hku {

IBarsLast::IBarsLast() : IndicatorImp("BARSLAST", 1) {}

IBarsLast::~IBarsLast() {}

bool IBarsLast::check() {
    return true;
}

void IBarsLast::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    if (total == m_discard + 1) {
        if (src[m_discard] != 0.0) {
            dst[m_discard] = 0.0;
        } else {
            m_discard = total;
        }
        return;
    }

    size_t pos = total;
    for (size_t i = total - 1; i != m_discard; i--) {
        if (src[i] != 0.0) {
            for (size_t j = i; j < pos; j++) {
                dst[j] = j - i;
            }
            pos = i;
        }
    }

    if (src[m_discard] != 0.0) {
        for (size_t i = m_discard; i < pos; i++) {
            dst[i] = i - m_discard;
        }
    } else {
        m_discard = pos;
    }
}

Indicator HKU_API BARSLAST() {
    return Indicator(make_shared<IBarsLast>());
}

} /* namespace hku */
