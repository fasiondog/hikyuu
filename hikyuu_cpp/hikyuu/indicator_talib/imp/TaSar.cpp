/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaSar.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaSar)
#endif

namespace hku {

TaSar::TaSar() : IndicatorImp("TA_SAR", 1) {
    setParam<double>("acceleration", 0.02);
    setParam<double>("maximum", 0.2);
}

TaSar::TaSar(const KData& k, double acceleration, double maximum) : IndicatorImp("TA_SAR", 1) {
    setParam<KData>("kdata", k);
    setParam<double>("acceleration", acceleration);
    setParam<double>("maximum", maximum);
    TaSar::_calculate(Indicator());
}

void TaSar::_checkParam(const string& name) const {
    if (name == "acceleration" || name == "maximum") {
        double p = getParam<double>(name);
        HKU_CHECK(p >= 0.0 && p <= 3.000000e+37, "{} must >= 0!", name);
    }
}

void TaSar::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    double acceleration = getParam<double>("acceleration");
    double maximum = getParam<double>("maximum");
    int back = TA_SAR_Lookback(acceleration, maximum);
    if (back < 0 || back >= total) {
        m_discard = total;
        return;
    }

    const KRecord* kptr = k.data();
    std::unique_ptr<double[]> buf = std::make_unique<double[]>(2 * total);
    double* high = buf.get();
    double* low = high + total;
    for (size_t i = 0; i < total; ++i) {
        high[i] = kptr[i].highPrice;
        low[i] = kptr[i].lowPrice;
    }

    auto* dst = this->data();
    m_discard = back;
    int outBegIdx;
    int outNbElement;
    TA_SAR(m_discard, total - 1, high, low, acceleration, maximum, &outBegIdx, &outNbElement,
           dst + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_SAR(double acceleration, double maximum) {
    auto p = make_shared<TaSar>();
    p->setParam<double>("acceleration", acceleration);
    p->setParam<double>("maximum", maximum);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API TA_SAR(const KData& k, double acceleration, double maximum) {
    return Indicator(make_shared<TaSar>(k, acceleration, maximum));
}

} /* namespace hku */
