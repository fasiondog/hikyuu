/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "OperatorSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorSignal)
#endif

namespace hku {

OperatorSignal::OperatorSignal() : SignalBase("SG_Operator") {
    m_ignore_cycle = true;
}
OperatorSignal::OperatorSignal(const string& name) : SignalBase(name) {
    m_ignore_cycle = true;
}

OperatorSignal::OperatorSignal(const string& name, const SignalPtr& sg1, const SignalPtr& sg2)
: SignalBase(name) {
    m_ignore_cycle = true;
    if (sg1) {
        m_sg1 = sg1->clone();
    }
    if (sg2) {
        m_sg2 = sg2->clone();
    }
}

OperatorSignal::~OperatorSignal() {}

void OperatorSignal::_reset() {
    if (m_sg1) {
        m_sg1->reset();
    }
    if (m_sg2) {
        m_sg2->reset();
    }
}

void OperatorSignal::sub_sg_calculate(SignalPtr& sg, const KData& kdata) {
    HKU_IF_RETURN(!sg, void());
    bool cycle = sg->getParam<bool>("cycle");
    if (m_kdata == kdata && !cycle) {
        sg->_calculate(kdata);
    } else if (m_kdata != Null<KData>() && m_kdata != kdata && cycle) {
        sg->startCycle(m_cycle_start, m_cycle_end);
        sg->_calculate(kdata);
    }
}

SignalPtr OperatorSignal::_clone() {
    return make_shared<OperatorSignal>(m_name, m_sg1, m_sg2);
}

} /* namespace hku */
