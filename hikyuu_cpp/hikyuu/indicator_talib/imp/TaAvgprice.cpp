/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaAvgprice.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaAvgprice)
#endif

namespace hku {

TaAvgprice::TaAvgprice() : IndicatorImp("TA_AVGPRICE", 1) {}

TaAvgprice::TaAvgprice(const KData& k) : IndicatorImp("TA_AVGPRICE", 1) {
    setParam<KData>("kdata", k);
    TaAvgprice::_calculate(Indicator());
}

void TaAvgprice::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);

    const KRecord* kptr = k.data();

    std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);
    double* open = buf.get();
    double* high = open + total;
    double* low = high + total;
    double* close = low + total;
    for (size_t i = 0; i < total; ++i) {
        open[i] = kptr[i].openPrice;
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
        close[i] = kptr[i].closePrice;
    }

    auto* dst = this->data();
    int outBegIdx;
    int outNbElement;
    TA_AVGPRICE(0, total - 1, open, high, low, close, &outBegIdx, &outNbElement, dst);
}

Indicator HKU_API TA_AVGPRICE() {
    return make_shared<TaAvgprice>()->calculate();
}

Indicator HKU_API TA_AVGPRICE(const KData& k) {
    return Indicator(make_shared<TaAvgprice>(k));
}

} /* namespace hku */
