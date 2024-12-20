/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaHtSine.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaHtSine)
#endif

namespace hku {

TaHtSine::TaHtSine() : IndicatorImp("TA_HT_SINE", 2) {}

void TaHtSine::_calculate(const Indicator& data) {
    size_t total = data.size();
    int lookback = TA_HT_SINE_Lookback();
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 2);
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);

    const double* src = data.data();

    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_HT_SINE(data.discard(), total - 1, src, &outBegIdx, &outNbElement, dst0 + m_discard,
               dst1 + m_discard);
    if (outBegIdx != m_discard) {
        memmove(dst0 + outBegIdx, dst0 + m_discard, sizeof(double) * outNbElement);
        memmove(dst1 + outBegIdx, dst1 + m_discard, sizeof(double) * outNbElement);
        double null_double = Null<double>();
        for (size_t i = m_discard; i < outBegIdx; ++i) {
            _set(null_double, i);
        }
        m_discard = outBegIdx;
    }
}

Indicator HKU_API TA_HT_SINE() {
    return Indicator(make_shared<TaHtSine>());
}

} /* namespace hku */
