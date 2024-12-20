/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "TaApo.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaApo)
#endif

namespace hku {

TaApo::TaApo() : IndicatorImp("TA_APO", 1) {
    setParam<int>("fast_n", 12);
    setParam<int>("slow_n", 26);
    setParam<int>("matype", 0);
}

void TaApo::_checkParam(const string& name) const {
    if (name == "fast_n" || name == "slow_n") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 2 && n <= 100000, "{} must >= 2 and <= 100000 ", name);
    } else if (name == "matype") {
        int matype = getParam<int>("matype");
        HKU_ASSERT(matype >= 0 && matype <= 8);
    }
}

void TaApo::_calculate(const Indicator& data) {
    int fast_n = getParam<int>("fast_n");
    int slow_n = getParam<int>("slow_n");
    TA_MAType matype = (TA_MAType)getParam<int>("matype");
    size_t total = data.size();
    int lookback = TA_APO_Lookback(fast_n, slow_n, matype);
    HKU_IF_RETURN(lookback < 0, void());

    _readyBuffer(total, 1);

    const double* src = data.data();
    auto* dst = this->data();

    m_discard = data.discard() + lookback;
    int outBegIdx;
    int outNbElement;
    TA_APO(data.discard(), total - 1, src, fast_n, slow_n, matype, &outBegIdx, &outNbElement,
           dst + m_discard);
    if (outBegIdx != m_discard) {
        memmove(dst + outBegIdx, dst + m_discard, sizeof(double) * outNbElement);
        double null_double = Null<double>();
        for (size_t i = m_discard; i < outBegIdx; ++i) {
            _set(null_double, i);
        }
        m_discard = outBegIdx;
    }
}

Indicator HKU_API TA_APO(int fast_n, int slow_n, int matype) {
    auto p = make_shared<TaApo>();
    p->setParam<int>("fast_n", fast_n);
    p->setParam<int>("slow_n", slow_n);
    p->setParam<int>("matype", matype);
    return Indicator(p);
}

} /* namespace hku */
