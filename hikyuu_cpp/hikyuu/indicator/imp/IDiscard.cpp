/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-31
 *      Author: fasiondog
 */

#include "IDiscard.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDiscard)
#endif

namespace hku {

IDiscard::IDiscard() : IndicatorImp("DISCARD") {
    setParam<int>("discard", 0);
}

IDiscard::~IDiscard() {}

void IDiscard::_checkParam(const string &name) const {
    if (name == "discard") {
        HKU_CHECK(getParam<int>(name) >= 0, "DISCARD: discard must >= 0!");
    }
}

void IDiscard::_calculate(const Indicator &data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    size_t result_num = data.getResultNumber();
    _readyBuffer(total, result_num);

    m_discard = (size_t)getParam<int>("discard");
    if (m_discard < data.discard()) {
        m_discard = data.discard();
    }

    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    for (size_t r = 0; r < result_num; ++r) {
        auto const *src = data.data(r);
        auto *dst = this->data(r);
        memcpy(dst + m_discard, src + m_discard, (total - m_discard) * sizeof(value_t));
    }
}

Indicator HKU_API DISCARD(int discard) {
    auto p = make_shared<IDiscard>();
    p->setParam<int>("discard", discard);
    return Indicator(p);
}

} /* namespace hku */
