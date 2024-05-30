/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-16
 *      Author: fasiondog
 */

#include "AndCondition.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::AndCondition)
#endif

namespace hku {

AndCondition::AndCondition() : ConditionBase("CN_And") {}

AndCondition::AndCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_And"), m_cond1(cond1), m_cond2(cond2) {}

AndCondition::~AndCondition() {}

void AndCondition::_calculate() {
    HKU_IF_RETURN(!m_cond1 || !m_cond2, void());
    m_cond1->setTM(m_tm);
    m_cond2->setTM(m_tm);
    m_cond1->setSG(m_sg);
    m_cond2->setSG(m_sg);
    m_cond1->setTO(m_kdata);
    m_cond2->setTO(m_kdata);

    size_t total = m_kdata.size();
    HKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = (data1[i] > 0.0 && data2[i] > 0.0) ? 1.0 : 0.0;
    }
}

void AndCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr AndCondition::_clone() {
    auto p = make_shared<AndCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HKU_API ConditionPtr operator&(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<AndCondition>(cond1, cond2);
}

}  // namespace hku