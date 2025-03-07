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

void DivEnvironment::_calculate() {
    HKU_IF_RETURN(!m_ev1 || !m_ev2, void());

    m_ev1->setQuery(m_query);
    m_ev2->setQuery(m_query);

    auto values1 = m_ev1->getValues();
    auto dates1 = values1.getDatetimeList();
    unordered_map<Datetime, price_t> value1_map;
    const auto* src1 = values1.data();
    for (size_t i = 0, total = dates1.size(); i < total; i++) {
        if (src1[i] != 0.0) {
            value1_map[dates1[i]] = src1[i];
        }
    }

    auto value2 = m_ev2->getValues();
    auto dates2 = value2.getDatetimeList();
    const auto* src2 = value2.data();
    for (size_t i = 0, total = dates2.size(); i < total; i++) {
        if (src2[i] != 0.0) {
            auto iter = value1_map.find(dates2[i]);
            if (iter != value1_map.end()) {
                _addValid(iter->first, iter->second / src2[i]);
            }
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
    return make_shared<DivEnvironment>(ev1, ev2);
}

}  // namespace hku