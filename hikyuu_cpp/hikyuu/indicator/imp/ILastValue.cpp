/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-04
 *      Author: fasiondog
 */

#include "ILastValue.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILastValue)
#endif

namespace hku {

ILastValue::ILastValue() : IndicatorImp("LASTVALUE", 1) {
    setParam<bool>("ignore_discard", false);  // 忽略输入指标的 discard
}

ILastValue::~ILastValue() {}

void ILastValue::_calculate(const Indicator &data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    bool ignore_discard = getParam<bool>("ignore_discard");
    if (!ignore_discard) {
        m_discard = data.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
    }

    value_t last_val = data[total - 1];
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = last_val;
    }
}

Indicator HKU_API LASTVALUE(bool ignore_discard) {
    auto p = make_shared<ILastValue>();
    p->setParam<bool>("ignore_discard", ignore_discard);
    return Indicator(p);
}

} /* namespace hku */
