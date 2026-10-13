/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#include "AndEnvironment.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::AndEnvironment)
#endif

namespace hku {

AndEnvironment::AndEnvironment() : EnvironmentBase("EV_And") {}

AndEnvironment::AndEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2)
: EnvironmentBase("EV_And") {
    if (ev1) {
        m_ev1 = ev1->clone();
    }
    if (ev2) {
        m_ev2 = ev2->clone();
    }
}

AndEnvironment::~AndEnvironment() {}

// two-pointer merge strategy (intersection only).
// Do NOT convert to the ev1-base-laydown style (Add/Sub/Or): that would wrongly keep ev1-only.
void AndEnvironment::_calculate() {
    HKU_IF_RETURN(!m_ev1 || !m_ev2, void());

    m_ev1->setQuery(m_query);
    m_ev2->setQuery(m_query);

    const auto& idx1 = m_ev1->dateIndexRef();
    const auto& val1 = m_ev1->valuesRef();
    const auto& idx2 = m_ev2->dateIndexRef();
    const auto& val2 = m_ev2->valuesRef();

    auto it1 = idx1.begin(), it2 = idx2.begin();
    while (it1 != idx1.end() && it2 != idx2.end()) {
        if (it1->first < it2->first) {
            ++it1;
        } else if (it2->first < it1->first) {
            ++it2;
        } else {
            if (val1[it1->second] > 0.0 && val2[it2->second] > 0.0) {
                m_date_index[it1->first] = m_values.size();
                m_values.push_back(1.0);
            }
            ++it1;
            ++it2;
        }
    }
}

void AndEnvironment::_reset() {
    if (m_ev1) {
        m_ev1->reset();
    }
    if (m_ev2) {
        m_ev2->reset();
    }
}

EnvironmentPtr AndEnvironment::_clone() {
    auto p = make_shared<AndEnvironment>();
    if (m_ev1) {
        p->m_ev1 = m_ev1->clone();
    }
    if (m_ev2) {
        p->m_ev2 = m_ev2->clone();
    }
    return p;
}

HKU_API EnvironmentPtr operator&(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2) {
    HKU_WARN_IF(!ev1 || !ev2, "AndEnvironment: one side is null, the result is always invalid.");
    return make_shared<AndEnvironment>(ev1, ev2);
}

}  // namespace hku