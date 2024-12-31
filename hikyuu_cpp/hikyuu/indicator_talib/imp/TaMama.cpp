/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaMama.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMama)
#endif

namespace hku {

TaMama::TaMama() : IndicatorImp("TA_MAMA", 2) {
    setParam<double>("fast_limit", 0.5);
    setParam<double>("slow_limit", 0.05);
}

void TaMama::_checkParam(const string& name) const {
    if (name == "fast_limit" || name == "slow_limit") {
        double limit = getParam<double>(name);
        HKU_CHECK(limit >= 0.01 && limit <= 0.99, "{} must be in [0.01, 0.99]!", name);
    }
}

void TaMama::_calculate(const Indicator& data) {
    double fast_limit = getParam<double>("fast_limit");
    double slow_limit = getParam<double>("slow_limit");
    size_t total = data.size();
    int lookback = TA_MAMA_Lookback(fast_limit, slow_limit);
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = data.discard() + lookback;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    const double* src = data.data();
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);

    int outBegIdx;
    int outNbElement;
    TA_MAMA(m_discard, total - 1, src, fast_limit, slow_limit, &outBegIdx, &outNbElement,
            dst0 + m_discard, dst1 + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_MAMA(double fast_limit, double slow_limit) {
    auto p = make_shared<TaMama>();
    p->setParam<double>("fast_limit", fast_limit);
    p->setParam<double>("slow_limit", slow_limit);
    return Indicator(p);
}

} /* namespace hku */
