/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMinIndex.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMinIndex)
#endif

namespace hku {

TaMinIndex::TaMinIndex() : IndicatorImp("TA_MININDEX", 1) {
    setParam<int>("n", 30);
}

void TaMinIndex::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 10000);
    }
}

void TaMinIndex::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    size_t total = data.size();
    int lookback = TA_MININDEX_Lookback(n);
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 1);

    const double* src = data.data();
    auto* dst = this->data(0);

    std::unique_ptr<int[]> buf = std::make_unique<int[]>(total);
    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_MININDEX(data.discard(), total - 1, src, n, &outBegIdx, &outNbElement,
                buf.get() + m_discard);
    if (outBegIdx > m_discard) {
        m_discard = outBegIdx;
    }
    for (size_t i = m_discard; i < total; i++) {
        dst[i] = buf[i];
    }
}

Indicator HKU_API TA_MININDEX(int n) {
    auto p = make_shared<TaMinIndex>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
