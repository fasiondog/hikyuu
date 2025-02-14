/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "MulValueSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MulValueSignal)
#endif

namespace hku {

void MulValueSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg || m_value == 0.0 || std::isnan(m_value), void());

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    m_sg->_calculate(kdata);
    for (size_t i = 0; i < total; ++i) {
        _addSignal(ks[i].datetime, m_sg->getValue(ks[i].datetime) * m_value);
    }
}

HKU_API SignalPtr operator*(const SignalPtr& sg, double value) {
    return make_shared<MulValueSignal>(sg, value);
}

HKU_API SignalPtr operator*(double value, const SignalPtr& sg) {
    return make_shared<MulValueSignal>(sg, value);
}

} /* namespace hku */
