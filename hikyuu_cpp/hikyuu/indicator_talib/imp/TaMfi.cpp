/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMfi.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMfi)
#endif

namespace hku {

TaMfi::TaMfi() : IndicatorImp("TA_MFI", 1) {
    setParam<int>("n", 14);
}

TaMfi::TaMfi(const KData& k, int n) : IndicatorImp("TA_MFI", 1) {
    setParam<KData>("kdata", k);
    setParam<int>("n", n);
    TaMfi::_calculate(Indicator());
}

void TaMfi::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 100000);
    }
}

void TaMfi::_calculate(const Indicator& data) {
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
    double* vol = close + total;
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
        close[i] = kptr[i].closePrice;
        vol[i] = kptr[i].transCount;
    }

    int n = getParam<int>("n");
    int back = TA_MFI_Lookback(n);
    HKU_IF_RETURN(back < 0, void());

    auto* dst = this->data();
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_MFI(0, total - 1, high, low, close, vol, n, &outBegIdx, &outNbElement, dst + back);
}

Indicator HKU_API TA_MFI(int n) {
    auto p = make_shared<TaMfi>();
    p->setParam<int>("n", n);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_MFI(const KData& k, int n) {
    return Indicator(make_shared<TaMfi>(k, n));
}

} /* namespace hku */
