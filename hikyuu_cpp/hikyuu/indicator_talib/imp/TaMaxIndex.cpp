/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMaxIndex.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMaxIndex)
#endif

namespace hku {

TaMaxIndex::TaMaxIndex() : IndicatorImp("TA_MAXINDEX", 1) {
    setParam<int>("n", 30);
}

void TaMaxIndex::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 10000);
    }
}

void TaMaxIndex::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    size_t total = data.size();
    int lookback = TA_MAXINDEX_Lookback(n);
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 1);

    const double* src = data.data();
    auto* dst = this->data(0);

    std::unique_ptr<int[]> buf = std::make_unique<int[]>(total);
    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_MAXINDEX(data.discard(), total - 1, src, n, &outBegIdx, &outNbElement,
                buf.get() + m_discard);
    if (outBegIdx > m_discard) {
        m_discard = outBegIdx;
    }
    for (size_t i = m_discard; i < total; i++) {
        dst[i] = buf[i];
    }
}

Indicator HKU_API TA_MAXINDEX(int n) {
    auto p = make_shared<TaMaxIndex>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
