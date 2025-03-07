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

void OrEnvironment::_calculate() {
    HKU_IF_RETURN(!m_ev1 && !m_ev2, void());

    if (m_ev1) {
        m_ev1->setQuery(m_query);
    }

    if (m_ev2) {
        m_ev2->setQuery(m_query);
    }

    if (m_ev1 && !m_ev2) {
        auto values = m_ev1->getValues();
        auto dates = values.getDatetimeList();
        size_t total = dates.size();
        m_values.reserve(total);
        for (size_t i = 0; i < total; i++) {
            if (values[i] > 0.0) {
                m_date_index[dates[i]] = m_values.size();
                m_values.push_back(1.0);
            }
        }
        return;
    }

    if (!m_ev1 && m_ev2) {
        auto values = m_ev2->getValues();
        auto dates = values.getDatetimeList();
        size_t total = dates.size();
        m_values.reserve(total);
        for (size_t i = 0; i < total; i++) {
            if (values[i] > 0.0) {
                m_date_index[dates[i]] = m_values.size();
                m_values.push_back(1.0);
            }
        }
        return;
    }

    auto values = m_ev1->getValues();
    auto dates = values.getDatetimeList();
    size_t total = dates.size();
    for (size_t i = 0; i < total; i++) {
        if (values[i] > 0.0) {
            m_date_index[dates[i]] = m_values.size();
            m_values.push_back(1.0);
        }
    }

    values = m_ev2->getValues();
    dates = values.getDatetimeList();
    total = dates.size();
    for (size_t i = 0; i < total; i++) {
        if (values[i] > 0.0) {
            auto iter = m_date_index.find(dates[i]);
            if (iter == m_date_index.end()) {
                m_date_index[dates[i]] = m_values.size();
                m_values.push_back(1.0);
            }
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
    return make_shared<OrEnvironment>(ev1, ev2);
}

}  // namespace hku