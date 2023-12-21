/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */

#include "BoolCondition.h"

namespace hku {

BoolCondition::BoolCondition() : ConditionBase("CN_Bool") {}

BoolCondition::BoolCondition(const Indicator& ind) : ConditionBase("CN_Bool"), m_ind(ind) {}

BoolCondition::~BoolCondition() {}

void BoolCondition::_reset() {}

ConditionPtr BoolCondition::_clone() {
    return make_shared<BoolCondition>(m_ind);
}

void BoolCondition::_calculate() {
    auto ds = m_kdata.getDatetimeList();
    m_ind.setContext(m_kdata);
    for (size_t i = m_ind.discard(), len = m_ind.size(); i < len; i++) {
        if (m_ind[i] > 0.) {
            _addValid(ds[i]);
        }
    }
}

CNPtr HKU_API CN_Bool(const Indicator& ind) {
    return make_shared<BoolCondition>(ind);
}

}  // namespace hku