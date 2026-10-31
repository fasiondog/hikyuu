/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#include "IIsInfa.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIsInfa)
#endif

namespace hku {

IIsInfa::IIsInfa() : IndicatorImp("ISINFA", 1) {}

IIsInfa::~IIsInfa() {}

void IIsInfa::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IIsInfa::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    value_t negative_inf = -std::numeric_limits<value_t>::infinity();
    for (size_t i = start_pos, total = data.size(); i < total; ++i) {
        dst[i] = (src[i] == negative_inf) ? 1.0 : 0.0;
    }
}

Indicator HKU_API ISINFA() {
    return Indicator(make_shared<IIsInfa>());
}

} /* namespace hku */
