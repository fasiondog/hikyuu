/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */

#include "BoolCondition.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::BoolCondition)
#endif

namespace hku {

BoolCondition::BoolCondition() : ConditionBase("CN_Bool") {}

BoolCondition::BoolCondition(const Indicator& ind) : ConditionBase("CN_Bool"), m_ind(ind) {}

BoolCondition::~BoolCondition() {}

ConditionPtr BoolCondition::_clone() {
    return make_shared<BoolCondition>(m_ind.clone());
}

void BoolCondition::_calculate() {
    auto ds = m_kdata.getDatetimeList();
    m_ind.setContext(m_kdata);
    for (size_t i = m_ind.discard(), len = m_ind.size(); i < len; i++) {
        if (!std::isnan(m_ind[i]) && m_ind[i] > 0.) {
            _addValid(ds[i]);
        }
    }
}

CNPtr HKU_API CN_Bool(const Indicator& ind) {
    return make_shared<BoolCondition>(ind);
}

}  // namespace hku