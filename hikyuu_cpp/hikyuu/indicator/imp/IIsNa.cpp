/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

#include "IIsNa.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIsNa)
#endif

namespace hku {

IIsNa::IIsNa() : IndicatorImp("ISNA", 1) {
    setParam<bool>("ignore_discard", false);
}

IIsNa::~IIsNa() {}

void IIsNa::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = getParam<bool>("ignore_discard") ? 0 : data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = std::isnan(src[i]);
    }
}

Indicator HKU_API ISNA(bool ignore_discard) {
    auto p = make_shared<IIsNa>();
    p->setParam<bool>("ignore_discard", ignore_discard);
    return Indicator(p);
}

} /* namespace hku */
