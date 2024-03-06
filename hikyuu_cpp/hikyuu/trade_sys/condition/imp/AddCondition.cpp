/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#include "AddCondition.h"

namespace hku {

AddCondition::AddCondition() : ConditionBase("CN_Add") {}

AddCondition::AddCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Add"), m_cond1(cond1), m_cond2(cond2) {}

AddCondition::~AddCondition() {}

void AddCondition::_calculate() {
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
        price_t const* data = m_cond1->data();
        for (size_t i = 0, total = m_cond1->size(); i < total; i++) {
            m_values[i] = data[i];
        }
        return;
    }

    if (!m_cond1 && m_cond2) {
        auto const* data = m_cond2->data();
        for (size_t i = 0, total = m_cond2->size(); i < total; i++) {
            m_values[i] = data[i];
        }
        return;
    }

    size_t total = m_kdata.size();
    HKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = data1[i] + data2[i];
    }
}

void AddCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr AddCondition::_clone() {
    AddCondition* p = new AddCondition();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return ConditionPtr(p);
}

HKU_API ConditionPtr operator+(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<AddCondition>(cond1, cond2);
}

}  // namespace hku