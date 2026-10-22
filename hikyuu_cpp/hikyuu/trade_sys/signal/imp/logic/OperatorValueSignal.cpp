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
        // The cycle attribute stays consistent with the child sg (a marker only); a cycle child is
        // a scheduled entry, so the combination inherits the non-alternating behavior too
        if (m_sg->getParam<bool>("cycle")) {
            setParam<bool>("alternate", false);
            setParam<bool>("cycle", true);
        }
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
    if (dynamic_cast<ManualSignal*>(m_sg.get())) {
        // A manual sub-signal holds pre-seeded signals; its empty _calculate keeps them intact
        m_sg->_calculate(kdata);
        return;
    }
    m_sg->setTO(kdata);
}

void OperatorValueSignal::setCycleDates(const DatetimeList& dates) {
    // Forward the declared rebalancing days to the cycle child first, so that it takes part in the
    // single full-range calculation of the composition. A manual sub-signal holds the pre-seeded
    // signals (must not be cleared by a recalculation); a non-cycle child does not consume the days
    if (m_sg && m_sg->getParam<bool>("cycle") && !dynamic_cast<ManualSignal*>(m_sg.get())) {
        m_sg->setCycleDates(dates);
    }
    SignalBase::setCycleDates(dates);
}

SignalPtr OperatorValueSignal::_clone() {
    return make_shared<OperatorValueSignal>(m_name, m_sg, m_value, m_mode);
}

} /* namespace hku */
