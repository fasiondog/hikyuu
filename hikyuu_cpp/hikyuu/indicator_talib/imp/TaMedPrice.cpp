/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMedPrice.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMedPrice)
#endif

namespace hku {

TaMedPrice::TaMedPrice() : IndicatorImp("TA_MEDPRICE", 1) {}

TaMedPrice::TaMedPrice(const KData& k) : IndicatorImp("TA_MEDPRICE", 1) {
    setParam<KData>("kdata", k);
    TaMedPrice::_calculate(Indicator());
}

void TaMedPrice::_calculate(const Indicator& data) {
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

    auto* dst = this->data();
    int outBegIdx;
    int outNbElement;
    TA_MEDPRICE(0, total - 1, high, low, &outBegIdx, &outNbElement, dst);
}

Indicator HKU_API TA_MEDPRICE() {
    return make_shared<TaMedPrice>()->calculate();
}

Indicator HKU_API TA_MEDPRICE(const KData& k) {
    return Indicator(make_shared<TaMedPrice>(k));
}

} /* namespace hku */
