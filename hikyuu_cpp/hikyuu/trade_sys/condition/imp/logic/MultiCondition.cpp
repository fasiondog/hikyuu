/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#include "MultiCondition.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiCondition)
#endif

namespace hku {

MultiCondition::MultiCondition() : ConditionBase("CN_Multi") {}

MultiCondition::MultiCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Multi") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

MultiCondition::~MultiCondition() {}

void MultiCondition::_calculate() {
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
        m_values[i] = data1[i] * data2[i];
    }
}

void MultiCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr MultiCondition::_clone() {
    auto p = make_shared<MultiCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HKU_API ConditionPtr operator*(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<MultiCondition>(cond1, cond2);
}

}  // namespace hku