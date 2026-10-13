/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#include "SubEnvironment.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::SubEnvironment)
#endif

namespace hku {

SubEnvironment::SubEnvironment() : EnvironmentBase("EV_Sub") {}

SubEnvironment::SubEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2)
: EnvironmentBase("EV_Sub") {
    if (ev1) {
        m_ev1 = ev1->clone();
    }
    if (ev2) {
        m_ev2 = ev2->clone();
    }
}

SubEnvironment::~SubEnvironment() {}

// ev1-base-laydown strategy (union-ish: keeps ev1-only, negates ev2-only).
// Do NOT convert to the And/Multi/Div intersection merge; that would drop ev1-only/ev2-only.
void SubEnvironment::_calculate() {
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
        m_values.resize(idx.size());
        size_t i = 0;
        for (const auto& [d, vi] : idx) {
            m_date_index[d] = i;
            m_values[i] = val[vi];
            ++i;
        }
        return;
    }

    if (!m_ev1 && m_ev2) {
        const auto& idx = m_ev2->dateIndexRef();
        const auto& val = m_ev2->valuesRef();
        m_values.resize(idx.size());
        size_t i = 0;
        for (const auto& [d, vi] : idx) {
            m_date_index[d] = i;
            m_values[i] = -val[vi];
            ++i;
        }
        return;
    }

    // both present: lay ev1 down as the base with NO map lookup, then align ev2 against it.
    // Intersections are subtracted by index (no find); only ev2-only dates take _addValid(-).
    const auto& idx1 = m_ev1->dateIndexRef();
    const auto& val1 = m_ev1->valuesRef();
    const auto& idx2 = m_ev2->dateIndexRef();
    const auto& val2 = m_ev2->valuesRef();

    size_t n1 = idx1.size();
    m_values.resize(n1);
    size_t i = 0;
    for (const auto& [d, vi] : idx1) {
        m_date_index[d] = i;
        m_values[i] = val1[vi];
        ++i;
    }

    // r1 is the rank of it1's date in idx1, i.e. our own m_values index assigned during the
    // laydown; never use it1->second (the child's storage index) as our own index.
    auto it1 = idx1.begin(), it2 = idx2.begin();
    size_t r1 = 0;
    while (it1 != idx1.end() && it2 != idx2.end()) {
        if (it1->first < it2->first) {
            ++it1;
            ++r1;
        } else if (it2->first < it1->first) {
            _addValid(it2->first, -val2[it2->second]);  // ev2-only
            ++it2;
        } else {
            m_values[r1] -= val2[it2->second];  // intersection, by our own rank
            ++it1;
            ++r1;
            ++it2;
        }
    }
    for (; it2 != idx2.end(); ++it2) {
        _addValid(it2->first, -val2[it2->second]);
    }
}

void SubEnvironment::_reset() {
    if (m_ev1) {
        m_ev1->reset();
    }
    if (m_ev2) {
        m_ev2->reset();
    }
}

EnvironmentPtr SubEnvironment::_clone() {
    auto p = make_shared<SubEnvironment>();
    if (m_ev1) {
        p->m_ev1 = m_ev1->clone();
    }
    if (m_ev2) {
        p->m_ev2 = m_ev2->clone();
    }
    return p;
}

HKU_API EnvironmentPtr operator-(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2) {
    HKU_WARN_IF(!ev1 || !ev2, "SubEnvironment: one side is null, the single-side mode will be used.");
    return make_shared<SubEnvironment>(ev1, ev2);
}

}  // namespace hku