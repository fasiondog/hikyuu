/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMama.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMama)
#endif

namespace hku {

TaMama::TaMama() : IndicatorImp("TA_MAMA", 2) {
    setParam<double>("fast_limit", 5.000000e-1);
    setParam<double>("slow_limit", 5.000000e-2);
}

void TaMama::_checkParam(const string& name) const {
    if (name == "fast_limit" || name == "slow_limit") {
        double limit = getParam<double>(name);
        HKU_ASSERT(limit >= 1.000000e-2 && limit <= 9.900000e-1);
    }
}

void TaMama::_calculate(const Indicator& data) {
    double fast_limit = getParam<double>("fast_limit");
    double slow_limit = getParam<double>("slow_limit");
    size_t total = data.size();
    int lookback = TA_MAMA_Lookback(fast_limit, slow_limit);
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 2);

    const double* src = data.data();
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);

    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_MAMA(data.discard(), total - 1, src, fast_limit, slow_limit, &outBegIdx, &outNbElement,
            dst0 + m_discard, dst1 + m_discard);
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

Indicator HKU_API TA_MAMA(double fast_limit, double slow_limit) {
    auto p = make_shared<TaMama>();
    p->setParam<double>("fast_limit", fast_limit);
    p->setParam<double>("slow_limit", slow_limit);
    return Indicator(p);
}

} /* namespace hku */
