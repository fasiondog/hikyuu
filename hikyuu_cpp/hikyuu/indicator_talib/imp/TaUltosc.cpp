/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaUltosc.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaUltosc)
#endif

namespace hku {

TaUltosc::TaUltosc() : IndicatorImp("TA_ULTOSC", 1) {
    setParam<int>("n1", 7);
    setParam<int>("n2", 14);
    setParam<int>("n3", 28);
}

TaUltosc::TaUltosc(const KData& k, int n1, int n2, int n3) : IndicatorImp("TA_ULTOSC", 1) {
    setParam<KData>("kdata", k);
    setParam<int>("n1", n1);
    setParam<int>("n2", n2);
    setParam<int>("n3", n3);
    TaUltosc::_calculate(Indicator());
}

void TaUltosc::_checkParam(const string& name) const {
    if (name == "n1" || name == "n2" || name == "n3") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 1 && n <= 100000, "{} must in [1, 100000]", name);
    }
}

void TaUltosc::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    int n1 = getParam<int>("n1");
    int n2 = getParam<int>("n2");
    int n3 = getParam<int>("n3");
    int back = TA_ULTOSC_Lookback(n1, n2, n3);
    if (back < 0 || back >= total) {
        m_discard = total;
        return;
    }

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

    auto* dst = this->data();
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_ULTOSC(m_discard, total - 1, high, low, close, n1, n2, n3, &outBegIdx, &outNbElement,
              dst + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_ULTOSC(int n1, int n2, int n3) {
    auto p = make_shared<TaUltosc>();
    p->setParam<int>("n1", n1);
    p->setParam<int>("n2", n2);
    p->setParam<int>("n2", n3);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_ULTOSC(const KData& k, int n1, int n2, int n3) {
    return Indicator(make_shared<TaUltosc>(k, n1, n2, n3));
}

} /* namespace hku */
