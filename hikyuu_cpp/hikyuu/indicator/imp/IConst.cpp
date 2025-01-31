/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-31
 *      Author: fasiondog
 */

#include "IConst.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IConst)
#endif

namespace hku {

IConst::IConst() : IndicatorImp("CONST", 1) {}

IConst::~IConst() {}

void IConst::_calculate(const Indicator &data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    value_t val = data[total - 1];

    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = val;
    }
}

Indicator HKU_API CONST() {
    return Indicator(make_shared<IConst>());
}

} /* namespace hku */
