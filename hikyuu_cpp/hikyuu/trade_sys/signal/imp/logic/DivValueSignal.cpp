/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "DivValueSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::DivValueSignal)
#endif

namespace hku {

DivValueSignal::DivValueSignal(double value, const SignalPtr& sg)
: OperatorValueSignal("SG_DivValue", sg, value, 1) {}

void DivValueSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg || m_value == 0.0, void());

    sub_sg_calculate(kdata);
    if (m_mode == 0) {
        sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy, double sell) {
            _addSignal(datetime, (buy + sell) / m_value);
        });
    } else {
        sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy, double sell) {
            double sg_value = buy + sell;
            if (sg_value != 0.0) {
                _addSignal(datetime, m_value / sg_value);
            }
        });
    }
}

HKU_API SignalPtr operator/(const SignalPtr& sg, double value) {
    return make_shared<DivValueSignal>(sg, value);
}

HKU_API SignalPtr operator/(double value, const SignalPtr& sg) {
    return make_shared<DivValueSignal>(value, sg);
}

} /* namespace hku */
