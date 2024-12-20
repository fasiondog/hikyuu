/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaAccbands.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaAccbands)
#endif

namespace hku {

TaAccbands::TaAccbands() : IndicatorImp("TA_ACCBANDS", 3) {
    setParam<int>("n", 20);
}

TaAccbands::TaAccbands(const KData& k, int n) : IndicatorImp("TA_ACCBANDS", 3) {
    setParam<KData>("kdata", k);
    setParam<int>("n", n);
    TaAccbands::_calculate(Indicator());
}

void TaAccbands::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 100000);
    }
}

void TaAccbands::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 3);

    const KRecord* kptr = k.data();

    std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);
    double* high = buf.get();
    double* low = high + total;
    double* close = low + total;
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
        close[i] = kptr[i].closePrice;
    }

    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    auto* dst2 = this->data(2);

    int n = getParam<int>("n");
    int back = TA_ACCBANDS_Lookback(n);
    HKU_IF_RETURN(back < 0, void());

    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_ACCBANDS(0, total - 1, high, low, close, n, &outBegIdx, &outNbElement, dst0 + back,
                dst1 + back, dst2 + back);
}

Indicator HKU_API TA_ACCBANDS() {
    return make_shared<TaAccbands>()->calculate();
}

Indicator HKU_API TA_ACCBANDS(const KData& k, int n) {
    return Indicator(make_shared<TaAccbands>(k, n));
}

} /* namespace hku */
