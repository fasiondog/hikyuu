/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaMa.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMa)
#endif

namespace hku {

TaMa::TaMa() : IndicatorImp("TA_MA", 1) {
    setParam<int>("n", 30);
    setParam<int>("matype", 0);
}

void TaMa::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>(name);
        HKU_ASSERT(n >= 2 && n <= 100000);
    } else if (name == "matype") {
        int matype = getParam<int>("matype");
        HKU_ASSERT(matype >= 0 && matype <= 8);
    }
}

void TaMa::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    TA_MAType matype = (TA_MAType)getParam<int>("matype");
    size_t total = data.size();
    int lookback = TA_MA_Lookback(n, matype);
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 1);

    const double* src = data.data();
    auto* dst = this->data();

    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_MA(data.discard(), total - 1, src, n, matype, &outBegIdx, &outNbElement, dst + m_discard);
    if (outBegIdx != m_discard) {
        memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);
        double null_double = Null<double>();
        for (size_t i = m_discard; i < outBegIdx; ++i) {
            _set(null_double, i);
        }
        m_discard = outBegIdx;
    }
}

Indicator HKU_API TA_MA(int n, int matype) {
    auto p = make_shared<TaMa>();
    p->setParam<int>("n", n);
    p->setParam<int>("matype", matype);
    return Indicator(p);
}

} /* namespace hku */
