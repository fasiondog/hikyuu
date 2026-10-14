/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#include <cmath>
#include "AddCondition.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::AddCondition)
#endif

namespace hku {

AddCondition::AddCondition() : ConditionBase("CN_Add") {}

AddCondition::AddCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Add") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

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

    price_t null_price = Null<price_t>();
    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = (std::isfinite(data1[i]) && std::isfinite(data2[i]))
                        ? data1[i] + data2[i]
                        : null_price;
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
    auto p = make_shared<AddCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HKU_API ConditionPtr operator+(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    HKU_WARN_IF(!cond1 || !cond2, "AddCondition: one side is null, the single-side mode will be used.");
    return make_shared<AddCondition>(cond1, cond2);
}

}  // namespace hku