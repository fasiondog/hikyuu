/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaVar.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaVar)
#endif

namespace hku {

TaVar::TaVar() : IndicatorImp("TA_VAR", 1) {
    setParam<int>("n", 5);
    setParam<double>("nbdev", 1.0);
}

void TaVar::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 1 && n <= 100000, "n must >= 1 and <= 100000 ");
    } else if (name == "nbdev") {
        double nbdev = getParam<double>("nbdev");
        HKU_ASSERT(nbdev >= -3.000000e+37 && nbdev <= 3.000000e+37);
    }
}

void TaVar::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    double nbdev = getParam<double>("nbdev");
    size_t total = data.size();
    int lookback = TA_VAR_Lookback(n, nbdev);
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 1);

    const double* src = data.data();
    auto* dst = this->data();

    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_VAR(data.discard(), total - 1, src, n, nbdev, &outBegIdx, &outNbElement, dst + m_discard);
    if (outBegIdx != m_discard) {
        memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);
        double null_double = Null<double>();
        for (size_t i = m_discard; i < outBegIdx; ++i) {
            _set(null_double, i);
        }
        m_discard = outBegIdx;
    }
}

Indicator HKU_API TA_VAR(int n, double nbdev) {
    auto p = make_shared<TaVar>();
    p->setParam<int>("n", n);
    p->setParam<double>("nbdev", nbdev);
    return Indicator(p);
}

} /* namespace hku */
