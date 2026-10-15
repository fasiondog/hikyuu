/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "AddSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::AddSignal)
#endif

namespace hku {

void AddSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg1 && !m_sg2, void());

    if (m_sg1 && !m_sg2) {
        sub_sg_calculate(m_sg1, kdata);
        sub_merged_calculate(kdata, [this](const Datetime& datetime, double buy1, double sell1,
                                           double, double) { _addSignal(datetime, buy1 + sell1); });
        return;
    }

    if (!m_sg1 && m_sg2) {
        sub_sg_calculate(m_sg2, kdata);
        sub_merged_calculate(kdata, [this](const Datetime& datetime, double, double, double buy2,
                                           double sell2) { _addSignal(datetime, buy2 + sell2); });
        return;
    }

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    sub_merged_calculate(
      kdata, [this](const Datetime& datetime, double buy1, double sell1, double buy2,
                    double sell2) { _addSignal(datetime, buy1 + sell1 + buy2 + sell2); });
}

HKU_API SignalPtr operator+(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<AddSignal>(sg1, sg2);
}

} /* namespace hku */
