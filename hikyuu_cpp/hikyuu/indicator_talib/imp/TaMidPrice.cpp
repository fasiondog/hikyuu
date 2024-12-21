/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMidPrice.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMidPrice)
#endif

namespace hku {

TaMidPrice::TaMidPrice() : IndicatorImp("TA_MIDPRICE", 1) {
    setParam<int>("n", 14);
}

TaMidPrice::TaMidPrice(const KData& k, int n) : IndicatorImp("TA_MIDPRICE", 1) {
    setParam<KData>("kdata", k);
    setParam<int>("n", n);
    TaMidPrice::_calculate(Indicator());
}

void TaMidPrice::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 100000);
    }
}

void TaMidPrice::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);

    const KRecord* kptr = k.data();

    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);
    double* high = buf.get();
    double* low = high + total;
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
    }

    int n = getParam<int>("n");
    int back = TA_MIDPRICE_Lookback(n);
    HKU_IF_RETURN(back < 0, void());

    auto* dst = this->data();
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_MIDPRICE(0, total - 1, high, low, n, &outBegIdx, &outNbElement, dst + back);
}

Indicator HKU_API TA_MIDPRICE(int n) {
    auto p = make_shared<TaMidPrice>();
    p->setParam<int>("n", n);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_MIDPRICE(const KData& k, int n) {
    return Indicator(make_shared<TaMidPrice>(k, n));
}

} /* namespace hku */
