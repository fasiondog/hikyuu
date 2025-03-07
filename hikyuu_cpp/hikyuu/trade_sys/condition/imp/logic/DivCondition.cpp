/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#include "DivCondition.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::DivCondition)
#endif

namespace hku {

DivCondition::DivCondition() : ConditionBase("CN_Div") {}

DivCondition::DivCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Div") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

DivCondition::~DivCondition() {}

void DivCondition::_calculate() {
    HKU_IF_RETURN(!m_cond1, void());

    m_cond1->setTM(m_tm);
    m_cond1->setSG(m_sg);
    m_cond1->setTO(m_kdata);

    price_t null_price = Null<price_t>();
    if (!m_cond2) {
        for (size_t i = 0, total = m_cond1->size(); i < total; i++) {
            m_values[i] = null_price;
        }
        return;
    }

    m_cond2->setTM(m_tm);
    m_cond2->setSG(m_sg);
    m_cond2->setTO(m_kdata);

    size_t total = m_kdata.size();
    HKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = data2[i] == 0.0 || std::isnan(data2[i]) ? null_price : data1[i] / data2[i];
    }
}

void DivCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr DivCondition::_clone() {
    auto p = make_shared<DivCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HKU_API ConditionPtr operator/(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<DivCondition>(cond1, cond2);
}

}  // namespace hku