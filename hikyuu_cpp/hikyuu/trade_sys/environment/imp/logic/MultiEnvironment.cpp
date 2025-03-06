/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#include "MultiEnvironment.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiEnvironment)
#endif

namespace hku {

MultiEnvironment::MultiEnvironment() : EnvironmentBase("EV_Multi") {}

MultiEnvironment::MultiEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2)
: EnvironmentBase("EV_Multi"), m_ev1(ev1), m_ev2(ev2) {}

MultiEnvironment::~MultiEnvironment() {}

void MultiEnvironment::_calculate() {
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
        m_values.resize(total);
        for (size_t i = 0; i < total; i++) {
            m_date_index[dates[i]] = i;
            m_values[i] = values[i];
        }
        return;
    }

    if (!m_ev1 && m_ev2) {
        auto values = m_ev2->getValues();
        auto dates = values.getDatetimeList();
        size_t total = dates.size();
        m_values.resize(total);
        for (size_t i = 0; i < total; i++) {
            m_date_index[dates[i]] = i;
            m_values[i] = values[i];
        }
        return;
    }

    auto values1 = m_ev1->getValues();
    auto dates1 = values1.getDatetimeList();
    unordered_map<Datetime, price_t> value1_map;
    const auto* src1 = values1.data();
    for (size_t i = 0, total = dates1.size(); i < total; i++) {
        value1_map[dates1[i]] = src1[i];
    }

    auto value2 = m_ev2->getValues();
    auto dates2 = value2.getDatetimeList();
    const auto* src2 = value2.data();
    for (size_t i = 0, total = dates2.size(); i < total; i++) {
        auto iter = value1_map.find(dates2[i]);
        if (iter != value1_map.end()) {
            _addValid(iter->first, iter->second * src2[i]);
        }
    }
}

void MultiEnvironment::_reset() {
    if (m_ev1) {
        m_ev1->reset();
    }
    if (m_ev2) {
        m_ev2->reset();
    }
}

EnvironmentPtr MultiEnvironment::_clone() {
    auto p = make_shared<MultiEnvironment>();
    if (m_ev1) {
        p->m_ev1 = m_ev1->clone();
    }
    if (m_ev2) {
        p->m_ev2 = m_ev2->clone();
    }
    return p;
}

HKU_API EnvironmentPtr operator*(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2) {
    return make_shared<MultiEnvironment>(ev1, ev2);
}

}  // namespace hku