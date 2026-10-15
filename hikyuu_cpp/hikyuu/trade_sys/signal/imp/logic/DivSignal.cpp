/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "DivSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::DivSignal)
#endif

namespace hku {

void DivSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg1 || !m_sg2, void());

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy1, double sell1,
                                       double buy2, double sell2) {
        double buy_value = (buy2 != 0.0) ? buy1 / buy2 : 0.0;
        double sell_value = (sell2 != 0.0) ? sell1 / sell2 : 0.0;
        _addSignal(datetime, buy_value - sell_value);
    });
}

HKU_API SignalPtr operator/(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<DivSignal>(sg1, sg2);
}

} /* namespace hku */
