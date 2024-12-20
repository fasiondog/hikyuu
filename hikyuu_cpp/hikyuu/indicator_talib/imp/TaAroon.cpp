/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaAroon.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaAroon)
#endif

namespace hku {

TaAroon::TaAroon() : IndicatorImp("TA_AROON", 2) {
    setParam<int>("n", 14);
}

TaAroon::TaAroon(const KData& k, int n) : IndicatorImp("TA_AROON", 2) {
    setParam<KData>("kdata", k);
    setParam<int>("n", n);
    TaAroon::_calculate(Indicator());
}

void TaAroon::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 100000);
    }
}

void TaAroon::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 2);

    const KRecord* kptr = k.data();

    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);
    double* high = buf.get();
    double* low = high + total;
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
    }

    int n = getParam<int>("n");
    int back = TA_AROON_Lookback(n);
    HKU_IF_RETURN(back < 0, void());

    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_AROON(0, total - 1, high, low, n, &outBegIdx, &outNbElement, dst0 + back, dst1 + back);
}

Indicator HKU_API TA_AROON(int n) {
    auto p = make_shared<TaAroon>();
    p->setParam<int>("n", n);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_AROON(const KData& k, int n) {
    return Indicator(make_shared<TaAroon>(k, n));
}

} /* namespace hku */
