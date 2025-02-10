/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "SubSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::SubSignal)
#endif

namespace hku {

void SubSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg1 && !m_sg2, void());

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    if (m_sg1 && !m_sg2) {
        sub_sg_calculate(m_sg1, kdata);
        for (size_t i = 0; i < total; ++i) {
            _addSignal(ks[i].datetime, m_sg1->getValue(ks[i].datetime));
        }
        return;
    }

    if (!m_sg1 && m_sg2) {
        sub_sg_calculate(m_sg2, kdata);
        for (size_t i = 0; i < total; i++) {
            double value = 0.0 - m_sg2->getValue(ks[i].datetime);
            _addSignal(ks[i].datetime, value);
        }
        return;
    }

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    for (size_t i = 0; i < total; ++i) {
        double value = m_sg1->getValue(ks[i].datetime) - m_sg2->getValue(ks[i].datetime);
        _addSignal(ks[i].datetime, value);
    }
}

HKU_API SignalPtr operator-(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<SubSignal>(sg1, sg2);
}

} /* namespace hku */
