/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "OperatorSignal.h"
#include "../ManualSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorSignal)
#endif

namespace hku {

OperatorSignal::OperatorSignal() : SignalBase("SG_Operator") {}
OperatorSignal::OperatorSignal(const string& name) : SignalBase(name) {}

OperatorSignal::OperatorSignal(const string& name, const SignalPtr& sg1, const SignalPtr& sg2)
: SignalBase(name) {
    if (sg1) {
        m_sg1 = sg1->clone();
    }
    if (sg2) {
        m_sg2 = sg2->clone();
    }
    // A cycle child is a scheduled entry: the combined signal inherits both the cycle marker and
    // the non-alternating behavior (a cycle signal never alternates), otherwise the later
    // rebalancing buys would be silently dropped by the alternation of the combination
    if ((m_sg1 && m_sg1->getParam<bool>("cycle")) || (m_sg2 && m_sg2->getParam<bool>("cycle"))) {
        setParam<bool>("alternate", false);
        setParam<bool>("cycle", true);
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
    if (dynamic_cast<ManualSignal*>(sg.get())) {
        // A manual sub-signal holds pre-seeded signals; its empty _calculate keeps them intact
        sg->_calculate(kdata);
        return;
    }
    sg->setTO(kdata);
}

void OperatorSignal::setCycleDates(const DatetimeList& dates) {
    // The declared rebalancing days belong to the whole composition: forward them to the cycle
    // children first, so that a cycle child (e.g. SG_Cycle) takes part in the single full-range
    // calculation. A manual sub-signal holds the pre-seeded signals, they must not be cleared by a
    // recalculation; a non-cycle child does not consume the days at all
    if (m_sg1 && m_sg1->getParam<bool>("cycle") && !dynamic_cast<ManualSignal*>(m_sg1.get())) {
        m_sg1->setCycleDates(dates);
    }
    if (m_sg2 && m_sg2->getParam<bool>("cycle") && !dynamic_cast<ManualSignal*>(m_sg2.get())) {
        m_sg2->setCycleDates(dates);
    }
    SignalBase::setCycleDates(dates);
}

void OperatorSignal::sub_merged_calculate(const KData& kdata, const MergedEmitFunc& emit) {
    auto const* ks = kdata.data();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    typedef std::map<Datetime, double> SigMap;
    // Up to four ordered candidate sources: the buy/sell tables of the two sub-signals
    const SigMap* maps[4] = {nullptr, nullptr, nullptr, nullptr};  // buy1, sell1, buy2, sell2
    if (m_sg1) {
        maps[0] = &m_sg1->getBuySignalMap();
        maps[1] = &m_sg1->getSellSignalMap();
    }
    if (m_sg2) {
        maps[2] = &m_sg2->getBuySignalMap();
        maps[3] = &m_sg2->getSellSignalMap();
    }

    SigMap::const_iterator its[4];
    SigMap::const_iterator ends[4];
    for (int i = 0; i < 4; ++i) {
        if (maps[i]) {
            its[i] = maps[i]->cbegin();
            ends[i] = maps[i]->cend();
        }
    }

    size_t bar_pos = 0;
    while (true) {
        // The next candidate datetime is the minimum head of the active tables
        bool has_candidate = false;
        Datetime candidate;
        for (int i = 0; i < 4; ++i) {
            if (maps[i] && its[i] != ends[i] && (!has_candidate || its[i]->first < candidate)) {
                candidate = its[i]->first;
                has_candidate = true;
            }
        }
        if (!has_candidate) {
            break;
        }

        double buy1 = 0.0, sell1 = 0.0, buy2 = 0.0, sell2 = 0.0;
        for (int i = 0; i < 4; ++i) {
            if (maps[i] && its[i] != ends[i] && its[i]->first == candidate) {
                switch (i) {
                    case 0:
                        buy1 = its[i]->second;
                        break;
                    case 1:
                        sell1 = its[i]->second;
                        break;
                    case 2:
                        buy2 = its[i]->second;
                        break;
                    case 3:
                        sell2 = its[i]->second;
                        break;
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
            emit(candidate, buy1, sell1, buy2, sell2);
        }
    }
}

SignalPtr OperatorSignal::_clone() {
    return make_shared<OperatorSignal>(m_name, m_sg1, m_sg2);
}

} /* namespace hku */
