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

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    for (size_t i = 0; i < total; ++i) {
        double buy_value1 = m_sg1->getBuyValue(ks[i].datetime);
        double buy_value2 = m_sg2->getBuyValue(ks[i].datetime);
        double buy_value = (buy_value2 != 0.0) ? buy_value1 / buy_value2 : 0.0;
        double sell_value1 = m_sg1->getSellValue(ks[i].datetime);
        double sell_value2 = m_sg2->getSellValue(ks[i].datetime);
        double sell_value = (sell_value2 != 0.0) ? sell_value1 / sell_value2 : 0.0;
        double value = buy_value - sell_value;
        _addSignal(ks[i].datetime, value);
    }
}

HKU_API SignalPtr operator/(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<DivSignal>(sg1, sg2);
}

} /* namespace hku */
