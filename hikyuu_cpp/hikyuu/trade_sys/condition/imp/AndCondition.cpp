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
    m_cond1->setTM(m_tm);
    m_cond2->setTM(m_tm);
    m_cond1->setSG(m_sg);
    m_cond2->setSG(m_sg);
    m_cond1->setTO(m_kdata);
    m_cond2->setTO(m_kdata);
    auto ds = m_kdata.getDatetimeList();
    for (const auto& date : ds) {
        if (m_cond1->isValid(date) && m_cond2->isValid(date)) {
            _addValid(date);
        }
    }
}

void AndCondition::_reset() {
    m_cond1->reset();
    m_cond2->reset();
}

ConditionPtr AndCondition::_clone() {
    AndCondition* p = new AndCondition();
    p->m_cond1 = m_cond1->clone();
    p->m_cond2 = m_cond2->clone();
    return ConditionPtr(p);
}

HKU_API ConditionPtr operator&(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    AndCondition* p = new AndCondition(cond1, cond2);
    return ConditionPtr(p);
}

}  // namespace hku