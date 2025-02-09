/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "MulSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MulSignal)
#endif

namespace hku {

void MulSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg1 || !m_sg2, void());

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    for (size_t i = 0; i < total; ++i) {
        double buy_value = m_sg1->getBuyValue(ks[i].datetime) * m_sg2->getBuyValue(ks[i].datetime);
        double sell_value =
          0.0 - m_sg1->getSellValue(ks[i].datetime) * m_sg2->getSellValue(ks[i].datetime);
        _addSignal(ks[i].datetime, buy_value + sell_value);
    }
}

HKU_API SignalPtr operator*(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<MulSignal>(sg1, sg2);
}

} /* namespace hku */
