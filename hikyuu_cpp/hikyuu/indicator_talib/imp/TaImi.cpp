/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaImi.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaImi)
#endif

namespace hku {

TaImi::TaImi() : IndicatorImp("TA_IMI", 1) {
    setParam<int>("n", 14);
}

TaImi::TaImi(const KData& k, int n) : IndicatorImp("TA_IMI", 1) {
    setParam<KData>("kdata", k);
    setParam<int>("n", n);
    TaImi::_calculate(Indicator());
}

void TaImi::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 100000);
    }
}

void TaImi::_calculate(const Indicator& data) {
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
    double* open = buf.get();
    double* close = open + total;
    for (size_t i = 0; i < total; ++i) {
        open[i] = kptr[i].openPrice;
        close[i] = kptr[i].closePrice;
    }

    int n = getParam<int>("n");
    int back = TA_IMI_Lookback(n);
    HKU_IF_RETURN(back < 0, void());

    auto* dst = this->data();
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_IMI(0, total - 1, open, close, n, &outBegIdx, &outNbElement, dst + back);
}

Indicator HKU_API TA_IMI(int n) {
    auto p = make_shared<TaImi>();
    p->setParam<int>("n", n);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_IMI(const KData& k, int n) {
    return Indicator(make_shared<TaImi>(k, n));
}

} /* namespace hku */
