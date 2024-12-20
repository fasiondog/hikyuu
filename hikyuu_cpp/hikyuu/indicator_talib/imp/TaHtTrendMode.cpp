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
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 1);
    auto* dst = this->data();
    const double* src = data.data();

    std::unique_ptr<int[]> buf = std::make_unique<int[]>(total);

    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_HT_TRENDMODE(data.discard(), total - 1, src, &outBegIdx, &outNbElement,
                    buf.get() + m_discard);
    if (outBegIdx > m_discard) {
        m_discard = outBegIdx;
    }
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = buf[i];
    }
}

Indicator HKU_API TA_HT_TRENDMODE() {
    return Indicator(make_shared<TaHtTrendMode>());
}

} /* namespace hku */
