/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#include "OrEnvironment.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OrEnvironment)
#endif

namespace hku {

OrEnvironment::OrEnvironment() : EnvironmentBase("EV_Or") {}

OrEnvironment::OrEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2)
: EnvironmentBase("EV_Or") {
    if (ev1) {
        m_ev1 = ev1->clone();
    }
    if (ev2) {
        m_ev2 = ev2->clone();
    }
}

OrEnvironment::~OrEnvironment() {}

// union-of-valid merge strategy: a date is emitted iff it is valid on at least one side.
// Do NOT convert to the ev1-base-laydown style (Add/Sub): that would add ev1's invalid dates
// as 0.0 to the output date set; do NOT convert to the intersection merge (And/Multi/Div)
// either, which would drop dates valid on only one side.
void OrEnvironment::_calculate() {
    HKU_IF_RETURN(!m_ev1 && !m_ev2, void());

    if (m_ev1) {
        m_ev1->setQuery(m_query);
    }

    if (m_ev2) {
        m_ev2->setQuery(m_query);
    }

    if (m_ev1 && !m_ev2) {
        const auto& idx = m_ev1->dateIndexRef();
        const auto& val = m_ev1->valuesRef();
        for (const auto& [d, vi] : idx) {
            if (val[vi] > 0.0) {
                m_date_index[d] = m_values.size();
                m_values.push_back(1.0);
            }
        }
        return;
    }

    if (!m_ev1 && m_ev2) {
        const auto& idx = m_ev2->dateIndexRef();
        const auto& val = m_ev2->valuesRef();
        for (const auto& [d, vi] : idx) {
            if (val[vi] > 0.0) {
                m_date_index[d] = m_values.size();
                m_values.push_back(1.0);
            }
        }
        return;
    }

    // both present: emit only dates valid on at least one side (union of valid), walking the
    // two ordered maps with two pointers so no map lookup is needed.
    const auto& idx1 = m_ev1->dateIndexRef();
    const auto& val1 = m_ev1->valuesRef();
    const auto& idx2 = m_ev2->dateIndexRef();
    const auto& val2 = m_ev2->valuesRef();

    auto it1 = idx1.begin(), it2 = idx2.begin();
    while (it1 != idx1.end() && it2 != idx2.end()) {
        if (it1->first < it2->first) {
            if (val1[it1->second] > 0.0) {
                m_date_index[it1->first] = m_values.size();
                m_values.push_back(1.0);
            }
            ++it1;
        } else if (it2->first < it1->first) {
            if (val2[it2->second] > 0.0) {
                m_date_index[it2->first] = m_values.size();
                m_values.push_back(1.0);
            }
            ++it2;
        } else {
            if (val1[it1->second] > 0.0 || val2[it2->second] > 0.0) {
                m_date_index[it1->first] = m_values.size();
                m_values.push_back(1.0);
            }
            ++it1;
            ++it2;
        }
    }
    for (; it1 != idx1.end(); ++it1) {
        if (val1[it1->second] > 0.0) {
            m_date_index[it1->first] = m_values.size();
            m_values.push_back(1.0);
        }
    }
    for (; it2 != idx2.end(); ++it2) {
        if (val2[it2->second] > 0.0) {
            m_date_index[it2->first] = m_values.size();
            m_values.push_back(1.0);
        }
    }
}

void OrEnvironment::_reset() {
    if (m_ev1) {
        m_ev1->reset();
    }
    if (m_ev2) {
        m_ev2->reset();
    }
}

EnvironmentPtr OrEnvironment::_clone() {
    auto p = make_shared<OrEnvironment>();
    if (m_ev1) {
        p->m_ev1 = m_ev1->clone();
    }
    if (m_ev2) {
        p->m_ev2 = m_ev2->clone();
    }
    return p;
}

HKU_API EnvironmentPtr operator|(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2) {
    HKU_WARN_IF(!ev1 || !ev2, "OrEnvironment: one side is null, the single-side mode will be used.");
    return make_shared<OrEnvironment>(ev1, ev2);
}

}  // namespace hku