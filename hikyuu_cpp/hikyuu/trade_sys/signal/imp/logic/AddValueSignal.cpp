/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "AddValueSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::AddValueSignal)
#endif

namespace hku {

void AddValueSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg, void());

    sub_sg_calculate(kdata);
    if (m_value == 0.0) {
        sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy, double sell) {
            _addSignal(datetime, buy + sell);
        });
    } else {
        // buy is recorded iff positive and sell iff negative, so the constant only shifts the
        // existing directions and the days without any entry stay silent, as in the per-bar loop
        sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy, double sell) {
            if (buy > 0.0) {
                buy += m_value;
            }
            if (sell < 0.0) {
                sell -= m_value;
            }
            _addSignal(datetime, buy + sell);
        });
    }
}

HKU_API SignalPtr operator+(const SignalPtr& sg, double value) {
    return make_shared<AddValueSignal>(sg, value);
}

HKU_API SignalPtr operator+(double value, const SignalPtr& sg) {
    return make_shared<AddValueSignal>(sg, value);
}

} /* namespace hku */
