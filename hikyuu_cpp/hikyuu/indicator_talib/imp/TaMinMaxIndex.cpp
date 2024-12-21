/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMinMaxIndex.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMinMaxIndex)
#endif

namespace hku {

TaMinMaxIndex::TaMinMaxIndex() : IndicatorImp("TA_MINMAXINDEX", 2) {
    setParam<int>("n", 30);
}

void TaMinMaxIndex::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 10000);
    }
}

void TaMinMaxIndex::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    size_t total = data.size();
    int lookback = TA_MINMAXINDEX_Lookback(n);
    HKU_IF_RETURN(lookback < 0, void());

    const double* src = data.data();
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);

    std::unique_ptr<int[]> buf = std::make_unique<int[]>(2 * total);
    int* buf0 = buf.get();
    int* buf1 = buf0 + total;
    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_MINMAXINDEX(data.discard(), total - 1, src, n, &outBegIdx, &outNbElement, buf0 + m_discard,
                   buf1 + m_discard);
    if (outBegIdx > m_discard) {
        m_discard = outBegIdx;
    }
    for (size_t i = m_discard; i < total; i++) {
        dst0[i] = buf0[i];
        dst1[i] = buf1[i];
    }
}

Indicator HKU_API TA_MINMAXINDEX(int n) {
    auto p = make_shared<TaMinMaxIndex>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
