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

AddSignal::AddSignal() : SignalBase("SG_Add") {}

AddSignal::AddSignal(const SignalPtr& sg1, const SignalPtr& sg2) : SignalBase("SG_Add") {
    if (sg1) {
        m_sg1 = sg1->clone();
    }
    if (sg2) {
        m_sg2 = sg2->clone();
    }
}

AddSignal::~AddSignal() {}

SignalPtr AddSignal::_clone() {
    return make_shared<AddSignal>(m_sg1, m_sg2);
}

void AddSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg1 && !m_sg2, void());

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    if (m_sg1 && !m_sg2) {
        m_sg1->_calculate(kdata);
        for (size_t i = 0; i < total; ++i) {
            double value = m_sg1->getValue(ks[i].datetime);
            _addSignal(ks[i].datetime, value);
        }
        return;
    }

    if (!m_sg1 && m_sg2) {
        m_sg2->_calculate(kdata);
        for (size_t i = 0; i < total; i++) {
            double value = m_sg2->getValue(ks[i].datetime);
            _addSignal(ks[i].datetime, value);
            return;
        }
    }

    m_sg1->_calculate(kdata);
    m_sg2->_calculate(kdata);
    for (size_t i = 0; i < total; ++i) {
        double value = m_sg1->getValue(ks[i].datetime) + m_sg2->getValue(ks[i].datetime);
        _addSignal(ks[i].datetime, value);
    }
}

SignalPtr HKU_API SG_Add(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<AddSignal>(sg1, sg2);
}

} /* namespace hku */
