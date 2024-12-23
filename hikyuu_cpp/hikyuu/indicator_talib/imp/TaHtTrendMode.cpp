/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaHtTrendMode.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaHtTrendMode)
#endif

namespace hku {

TaHtTrendMode::TaHtTrendMode() : IndicatorImp("TA_HT_TRENDMODE", 1) {}

void TaHtTrendMode::_calculate(const Indicator& data) {
    size_t total = data.size();
    int lookback = TA_HT_TRENDMODE_Lookback();
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = data.discard() + lookback;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto* dst = this->data();
    const double* src = data.data();

    std::unique_ptr<int[]> buf = std::make_unique<int[]>(total);

    int outBegIdx;
    int outNbElement;
    TA_HT_TRENDMODE(m_discard, total - 1, src, &outBegIdx, &outNbElement, buf.get());
    HKU_ASSERT((outBegIdx + outNbElement) <= total);
    if (outBegIdx > m_discard) {
        m_discard = outBegIdx;
    }
    dst = dst + m_discard;
    for (size_t i = 0; i < outNbElement; ++i) {
        dst[i] = buf[i];
    }
}

Indicator HKU_API TA_HT_TRENDMODE() {
    return Indicator(make_shared<TaHtTrendMode>());
}

} /* namespace hku */
