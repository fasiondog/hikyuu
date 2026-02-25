/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-22
 *      Author: fasiondog
 */

#include "IRefX.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRefX)
#endif

namespace hku {

IRefX::IRefX() : IndicatorImp("REFX", 1) {
    setParam<int>("n", 1);
}

IRefX::~IRefX() {}

void IRefX::_calculate(const Indicator &data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    if (0 == n) {
        m_discard = data.discard();
        const auto *src = data.data() + m_discard;
        auto *dst = this->data() + m_discard;
        memcpy(dst, src, (total - m_discard) * sizeof(value_t));
        return;

    } else if (n > 0) {
        m_discard = data.discard() + n;
        if (m_discard >= total) {
            m_discard = total;
            return;
        }

        const auto *src = data.data() + data.discard();
        auto *dst = this->data() + m_discard;
        memcpy(dst, src, (total - m_discard) * sizeof(value_t));
        return;

    } else {
        size_t absn = std::abs(n);
        if (absn >= total) {
            m_discard = total;
            return;
        }

        int64_t startix = data.discard() - absn;
        size_t len = total - data.discard();
        if (startix < 0) {
            m_discard = 0;
            len = total - absn;
        } else {
            m_discard = startix;
        }

        const auto *src = data.data() + total - len;
        auto *dst = this->data() + m_discard;
        memcpy(dst, src, len * sizeof(value_t));
        return;
    }
}

Indicator HKU_API REFX(int n) {
    IndicatorImpPtr p = make_shared<IRefX>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
