/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMacdfix.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMacdfix)
#endif

namespace hku {

TaMacdfix::TaMacdfix() : IndicatorImp("TA_MACDFIX", 3) {
    setParam<int>("n", 9);
}

void TaMacdfix::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 1 && n <= 100000);
    }
}

void TaMacdfix::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    size_t total = data.size();
    int lookback = TA_MACDFIX_Lookback(n);
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 3);

    const double* src = data.data();
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    auto* dst2 = this->data(2);

    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_MACDFIX(data.discard(), total - 1, src, n, &outBegIdx, &outNbElement, dst0 + m_discard,
               dst1 + m_discard, dst2 + m_discard);
    if (outBegIdx != m_discard) {
        memmove(dst0 + outBegIdx, dst0 + m_discard, sizeof(double) * outNbElement);
        memmove(dst1 + outBegIdx, dst1 + m_discard, sizeof(double) * outNbElement);
        memmove(dst2 + outBegIdx, dst2 + m_discard, sizeof(double) * outNbElement);
        double null_double = Null<double>();
        for (size_t i = m_discard; i < outBegIdx; ++i) {
            _set(null_double, i);
        }
        m_discard = outBegIdx;
    }
}

Indicator HKU_API TA_MACDFIX(int n) {
    auto p = make_shared<TaMacdfix>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
