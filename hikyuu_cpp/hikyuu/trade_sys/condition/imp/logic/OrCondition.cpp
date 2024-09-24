/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#include "OrCondition.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OrCondition)
#endif

namespace hku {

OrCondition::OrCondition() : ConditionBase("CN_Or") {}

OrCondition::OrCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Or"), m_cond1(cond1), m_cond2(cond2) {}

OrCondition::~OrCondition() {}

void OrCondition::_calculate() {
    HKU_IF_RETURN(!m_cond1 && !m_cond2, void());

    if (m_cond1) {
        m_cond1->setTM(m_tm);
        m_cond1->setSG(m_sg);
        m_cond1->setTO(m_kdata);
    }

    if (m_cond2) {
        m_cond2->setTM(m_tm);
        m_cond2->setSG(m_sg);
        m_cond2->setTO(m_kdata);
    }

    if (m_cond1 && !m_cond2) {
        auto const* data = m_cond1->data();
        for (size_t i = 0, total = m_cond1->size(); i < total; i++) {
            if (data[i] > 0.0) {
                m_values[i] = 1.0;
            }
        }
        return;
    }

    if (!m_cond1 && m_cond2) {
        auto const* data = m_cond2->data();
        for (size_t i = 0, total = m_cond2->size(); i < total; i++) {
            if (data[i] > 0.0) {
                m_values[i] = 1.0;
            }
        }
        return;
    }

    size_t total = m_kdata.size();
    HKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        if (data1[i] > 0. || data2[i] > 0.) {
            m_values[i] = 1.0;
        }
    }
}

void OrCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr OrCondition::_clone() {
    auto p = make_shared<OrCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HKU_API ConditionPtr operator|(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<OrCondition>(cond1, cond2);
}

}  // namespace hku