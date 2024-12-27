/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaAdosc.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaAdosc)
#endif

namespace hku {

TaAdosc::TaAdosc() : IndicatorImp("TA_ADOSC", 1) {
    setParam<int>("fast_n", 3);
    setParam<int>("slow_n", 10);
}

TaAdosc::TaAdosc(const KData& k, int fast_n, int slow_n) : IndicatorImp("TA_ADOSC", 1) {
    setParam<KData>("kdata", k);
    setParam<int>("fast_n", fast_n);
    setParam<int>("slow_n", slow_n);
    TaAdosc::_calculate(Indicator());
}

void TaAdosc::_checkParam(const string& name) const {
    if (name == "fast_n") {
        int fast_n = getParam<int>("fast_n");
        HKU_ASSERT(fast_n >= 2 && fast_n <= 100000);
    } else if (name == "slow_n") {
        int slow_n = getParam<int>("slow_n");
        HKU_ASSERT(slow_n >= 2 && slow_n <= 100000);
    }
}

void TaAdosc::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);

    int fast_n = getParam<int>("fast_n");
    int slow_n = getParam<int>("slow_n");
    int back = TA_ADOSC_Lookback(fast_n, slow_n);
    if (back < 0 || back >= total) {
        m_discard = total;
        return;
    }

    const KRecord* kptr = k.data();
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);
    double* high = buf.get();
    double* low = high + total;
    double* close = low + total;
    double* vol = close + total;
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
        close[i] = kptr[i].closePrice;
        vol[i] = kptr[i].transCount;
    }

    m_discard = back;
    auto* dst = this->data();
    int outBegIdx;
    int outNbElement;
    TA_ADOSC(m_discard, total - 1, high, low, close, vol, fast_n, slow_n, &outBegIdx, &outNbElement,
             dst + m_discard);
    HKU_ASSERT(m_discard == outBegIdx);
}

Indicator HKU_API TA_ADOSC(int fast_n, int slow_n) {
    auto p = make_shared<TaAdosc>();
    p->setParam<int>("fast_n", fast_n);
    p->setParam<int>("slow_n", slow_n);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_ADOSC(const KData& k, int fast_n, int slow_n) {
    return Indicator(make_shared<TaAdosc>(k, fast_n, slow_n));
}

} /* namespace hku */
