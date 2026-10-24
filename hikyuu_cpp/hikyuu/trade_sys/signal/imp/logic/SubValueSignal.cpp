/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "SubValueSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::SubValueSignal)
#endif

namespace hku {

SubValueSignal::SubValueSignal(double value, const SignalPtr& sg)
: OperatorValueSignal("SG_SubValue", sg, value, 1) {}

void SubValueSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg, void());

    sub_sg_calculate(kdata);
    if (m_value == 0.0) {
        // mode 0: sg - 0 keeps the values, mode 1: 0 - sg negates them
        double sign = (m_mode == 0) ? 1.0 : -1.0;
        sub_merged_calculate(kdata,
                             [this, sign](const Datetime& datetime, double buy, double sell) {
                                 _addSignal(datetime, sign * (buy + sell));
                             });
    } else if (m_mode == 0) {
        sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy, double sell) {
            if (buy > 0.0) {
                buy -= m_value;
            }
            if (sell < 0.0) {
                sell -= m_value;
            }
            _addSignal(datetime, buy + sell);
        });
    } else {
        sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy, double sell) {
            if (buy > 0.0) {
                buy = m_value - buy;
            }
            if (sell < 0.0) {
                sell = m_value - sell;
            }
            _addSignal(datetime, buy + sell);
        });
    }
}

HKU_API SignalPtr operator-(const SignalPtr& sg, double value) {
    return make_shared<SubValueSignal>(sg, value);
}

HKU_API SignalPtr operator-(double value, const SignalPtr& sg) {
    return make_shared<SubValueSignal>(value, sg);
}

} /* namespace hku */
