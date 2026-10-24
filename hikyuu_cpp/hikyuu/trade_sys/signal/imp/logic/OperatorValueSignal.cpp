/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "OperatorValueSignal.h"
#include "../ManualSignal.h"

#include <functional>

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
    HKU_CHECK(!std::isinf(m_value), "The value must be finite! value: {}, mode: {}", m_value,
              m_mode);
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

void OperatorValueSignal::sub_merged_calculate(const KData& kdata, const MergedEmitFunc& emit) {
    auto const* ks = kdata.data();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0 || !m_sg, void());

    typedef std::map<Datetime, double> SigMap;
    // Two ordered candidate sources: the buy and the sell tables of the sub-signal
    const SigMap* maps[2] = {&m_sg->getBuySignalMap(), &m_sg->getSellSignalMap()};

    SigMap::const_iterator its[2];
    SigMap::const_iterator ends[2];
    for (int i = 0; i < 2; ++i) {
        its[i] = maps[i]->cbegin();
        ends[i] = maps[i]->cend();
    }

    size_t bar_pos = 0;
    while (true) {
        // The next candidate datetime is the minimum head of the active tables
        bool has_candidate = false;
        Datetime candidate;
        for (int i = 0; i < 2; ++i) {
            if (its[i] != ends[i] && (!has_candidate || its[i]->first < candidate)) {
                candidate = its[i]->first;
                has_candidate = true;
            }
        }
        if (!has_candidate) {
            break;
        }

        double buy = 0.0, sell = 0.0;
        for (int i = 0; i < 2; ++i) {
            if (its[i] != ends[i] && its[i]->first == candidate) {
                if (i == 0) {
                    buy = its[i]->second;
                } else {
                    sell = its[i]->second;
                }
                ++its[i];
            }
        }

        // The old per-bar loop only touches real bars; a manual pre-seeded entry may sit at a
        // non-bar datetime or outside the current window, it must be dropped the same way
        while (bar_pos < total && ks[bar_pos].datetime < candidate) {
            ++bar_pos;
        }
        if (bar_pos < total && ks[bar_pos].datetime == candidate) {
            emit(candidate, buy, sell);
        }
    }
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
