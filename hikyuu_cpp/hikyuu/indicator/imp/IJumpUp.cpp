/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#include "IJumpUp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IJumpUp)
#endif

namespace hku {

IJumpUp::IJumpUp() : IndicatorImp("JUMPUP", 1) {}

IJumpUp::~IJumpUp() {}

void IJumpUp::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard() + 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    const auto* src = ind.data();
    auto* dst = data();
    for (size_t i = m_discard; i < total; ++i) {
        if (src[i - 1] <= 0.0 && src[i] > 0.0) {
            dst[i] = 1.0;
        } else {
            dst[i] = 0.0;
        }
    }
}

Indicator HKU_API JUMPUP() {
    return Indicator(make_shared<IJumpUp>());
}

} /* namespace hku */
