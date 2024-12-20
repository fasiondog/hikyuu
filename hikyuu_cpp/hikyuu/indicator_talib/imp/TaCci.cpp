/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaCci.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaCci)
#endif

namespace hku {

TaCci::TaCci() : IndicatorImp("TA_CCI", 1) {
    setParam<int>("n", 14);
}

TaCci::TaCci(const KData& k, int n) : IndicatorImp("TA_CCI", 1) {
    setParam<KData>("kdata", k);
    setParam<int>("n", n);
    TaCci::_calculate(Indicator());
}

void TaCci::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 1 && n <= 100000);
    }
}

void TaCci::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);

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

    int n = getParam<int>("n");
    int back = TA_CCI_Lookback(n);
    HKU_IF_RETURN(back < 0, void());

    auto* dst = this->data();
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_CCI(0, total - 1, high, low, close, n, &outBegIdx, &outNbElement, dst + back);
}

Indicator HKU_API TA_CCI(int n) {
    auto p = make_shared<TaCci>();
    p->setParam<int>("n", n);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_CCI(const KData& k, int n) {
    return Indicator(make_shared<TaCci>(k, n));
}

} /* namespace hku */
