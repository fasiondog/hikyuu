/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#include "DivEnvironment.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::DivEnvironment)
#endif

namespace hku {

DivEnvironment::DivEnvironment() : EnvironmentBase("EV_Div") {}

DivEnvironment::DivEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2)
: EnvironmentBase("EV_Div") {
    if (ev1) {
        m_ev1 = ev1->clone();
    }
    if (ev2) {
        m_ev2 = ev2->clone();
    }
}

DivEnvironment::~DivEnvironment() {}

// two-pointer merge strategy (intersection only, ratio of both sides).
// Do NOT convert to the ev1-base-laydown style (Add/Sub/Or): that would wrongly keep ev1-only.
void DivEnvironment::_calculate() {
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
            price_t v1 = val1[it1->second];
            price_t v2 = val2[it2->second];
            if (v1 != 0.0 && v2 != 0.0) {
                m_date_index[it1->first] = m_values.size();
                m_values.push_back(v1 / v2);
            }
            ++it1;
            ++it2;
        }
    }
}

void DivEnvironment::_reset() {
    if (m_ev1) {
        m_ev1->reset();
    }
    if (m_ev2) {
        m_ev2->reset();
    }
}

EnvironmentPtr DivEnvironment::_clone() {
    auto p = make_shared<DivEnvironment>();
    if (m_ev1) {
        p->m_ev1 = m_ev1->clone();
    }
    if (m_ev2) {
        p->m_ev2 = m_ev2->clone();
    }
    return p;
}

HKU_API EnvironmentPtr operator/(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2) {
    HKU_WARN_IF(!ev1 || !ev2, "DivEnvironment: one side is null, the result is always invalid.");
    return make_shared<DivEnvironment>(ev1, ev2);
}

}  // namespace hku