/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include <cmath>
#include "OrSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OrSignal)
#endif

namespace hku {

void OrSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg1 && !m_sg2, void());

    auto emit = [this](const Datetime& datetime, double buy1, double sell1, double buy2,
                       double sell2) {
        double value = buy1 + sell1 + buy2 + sell2;
        if (std::isfinite(value) && value > 0.0) {
            _addBuySignal(datetime);
        } else if (std::isfinite(value) && value < 0.0) {
            _addSellSignal(datetime);
        }
    };

    if (m_sg1 && !m_sg2) {
        sub_sg_calculate(m_sg1, kdata);
        sub_merged_calculate(kdata, emit);
        return;
    }

    if (!m_sg1 && m_sg2) {
        sub_sg_calculate(m_sg2, kdata);
        sub_merged_calculate(kdata, emit);
        return;
    }

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    sub_merged_calculate(kdata, emit);
}

HKU_API SignalPtr operator|(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<OrSignal>(sg1, sg2);
}

} /* namespace hku */
