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
    HKU_IF_RETURN(!m_sg || m_value == 0.0, void());

    sub_sg_calculate(kdata);
    sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy, double sell) {
        _addSignal(datetime, (buy + sell) * m_value);
    });
}

HKU_API SignalPtr operator*(const SignalPtr& sg, double value) {
    return make_shared<MulValueSignal>(sg, value);
}

HKU_API SignalPtr operator*(double value, const SignalPtr& sg) {
    return make_shared<MulValueSignal>(sg, value);
}

} /* namespace hku */
