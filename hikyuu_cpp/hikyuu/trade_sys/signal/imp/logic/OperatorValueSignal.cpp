/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "OperatorValueSignal.h"
#include "../ManualSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorValueSignal)
#endif

namespace hku {

OperatorValueSignal::OperatorValueSignal() : SignalBase("SG_OperatorValue") {}
OperatorValueSignal::OperatorValueSignal(const string& name) : SignalBase(name) {}

OperatorValueSignal::OperatorValueSignal(const string& name, const SignalPtr& sg, double value,
                                         int mode)
: SignalBase(name), m_value(value), m_mode(mode) {
    if (sg) {
        m_sg = sg->clone();
        // The cycle attribute stays consistent with the child sg
        setParam<bool>("cycle", m_sg->getParam<bool>("cycle"));
    }
    if (std::isnan(m_value)) {
        m_value = 0.0;
    }
}

OperatorValueSignal::~OperatorValueSignal() {}

void OperatorValueSignal::_reset() {
    if (m_sg) {
        m_sg->reset();
    }
}

void OperatorValueSignal::sub_sg_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg, void());
    if (m_sg->getParam<bool>("cycle")) {
        // A cycle sub-signal is driven by startCycle within its own cycle window only
        HKU_IF_RETURN(m_cycle_end.isNull(), void());
        m_sg->setTO(m_kdata);
        m_sg->startCycle(m_cycle_start, m_cycle_end);
        return;
    }
    if (dynamic_cast<ManualSignal*>(m_sg.get())) {
        // A manual sub-signal holds pre-seeded signals; its empty _calculate keeps them intact
        m_sg->_calculate(kdata);
        return;
    }
    m_sg->setTO(kdata);
}

SignalPtr OperatorValueSignal::_clone() {
    return make_shared<OperatorValueSignal>(m_name, m_sg, m_value, m_mode);
}

} /* namespace hku */
