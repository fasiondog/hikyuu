/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "AndSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::AndSignal)
#endif

namespace hku {

void AndSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg1 || !m_sg2, void());

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    for (size_t i = 0; i < total; ++i) {
        double buy_value = m_sg1->getBuyValue(ks[i].datetime) * m_sg2->getBuyValue(ks[i].datetime);
        double sell_value =
          0.0 - m_sg1->getSellValue(ks[i].datetime) * m_sg2->getSellValue(ks[i].datetime);
        auto value = buy_value + sell_value;
        if (value > 0.0) {
            _addBuySignal(ks[i].datetime);
        } else if (value < 0.0) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

HKU_API SignalPtr operator&(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<AndSignal>(sg1, sg2);
}

} /* namespace hku */
