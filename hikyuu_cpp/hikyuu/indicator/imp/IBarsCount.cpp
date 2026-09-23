/*
 * IBarsCount.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-12
 *      Author: fasiondog
 */

#include "IBarsCount.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBarsCount)
#endif

namespace hku {

IBarsCount::IBarsCount() : IndicatorImp("BARSCOUNT", 1) {}

IBarsCount::~IBarsCount() {}

void IBarsCount::_calculate(const Indicator& ind) {
    KData k = ind.getContext();
    Stock stk = k.getStock();

    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto* dst = this->data();

    // If there is no context, take the discard of this indicator for the calculation directly
    if (stk.isNull()) {
        for (size_t i = m_discard; i < total; i++) {
            dst[i] = i + 1 - m_discard;
        }

        return;
    }

    // For the 1-minute line, get the number of the trading minutes of the day
    KQuery q = k.getQuery();
    auto const* krecords = k.data();
    if (q.kType() == KQuery::MIN) {
        Datetime pre_d = krecords[m_discard].datetime.startOfDay();
        size_t count = 0;
        for (size_t i = m_discard; i < total; i++) {
            Datetime d = krecords[i].datetime.startOfDay();
            if (d != pre_d) {
                pre_d = d;
                count = 0;
            }
            dst[i] = ++count;
        }

        return;
    }

    // Get the total number of the trading days since the listing
    size_t k_start_pos = k.startPos();
    if (k_start_pos != Null<size_t>()) {
        for (size_t i = m_discard; i < total; i++) {
            dst[i] = 1 + k_start_pos + i;
        }
    }

    return;
}

Indicator HKU_API BARSCOUNT() {
    return Indicator(make_shared<IBarsCount>());
}

} /* namespace hku */
