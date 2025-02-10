/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "OperatorValueSignal.h"

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
        // cycle 属性保持和子 sg 一致
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

SignalPtr OperatorValueSignal::_clone() {
    return make_shared<OperatorValueSignal>(m_name, m_sg, m_value);
}

} /* namespace hku */
