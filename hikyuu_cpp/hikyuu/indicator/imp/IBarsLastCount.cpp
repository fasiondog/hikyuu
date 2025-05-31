/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-01
 *      Author: fasiondog
 */

#include "IBarsLastCount.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBarsLastCount)
#endif

namespace hku {

IBarsLastCount::IBarsLastCount() : IndicatorImp("BARSLASTCOUNT", 1) {}

IBarsLastCount::~IBarsLastCount() {}

void IBarsLastCount::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();
    if (src[m_discard] > 0.0) {
        dst[m_discard] = 1.0;
    } else {
        dst[m_discard] = 0;
    }
    for (size_t i = m_discard; i < total; ++i) {
        if (src[i] > 0.0) {
            dst[i] = dst[i - 1] + 1.0;
        } else {
            dst[i] = 0.0;
        }
    }
}

Indicator HKU_API BARSLASTCOUNT() {
    return Indicator(make_shared<IBarsLastCount>());
}

} /* namespace hku */
