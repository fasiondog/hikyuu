/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */

#include <cmath>
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
    m_ind.setContext(m_kdata);
    auto const* ind_data = m_ind.data();
    size_t discard = m_ind.discard();
    size_t total = m_ind.size();
    if (total == m_kdata.size()) {
        const auto* kdata_ptr = m_kdata.data();
        for (size_t i = discard; i < total; i++) {
            if (std::isfinite(ind_data[i]) && ind_data[i] > 0.) {
                _addValid(kdata_ptr[i].datetime);
            }
        }
    } else {
        DatetimeList dates = m_ind.getDatetimeList();
        for (size_t i = discard; i < total; i++) {
            if (std::isfinite(ind_data[i]) && ind_data[i] > 0.) {
                _addValid(dates[i]);
            }
        }
    }
}

CNPtr HKU_API CN_Bool(const Indicator& ind) {
    return make_shared<BoolCondition>(ind);
}

}  // namespace hku