/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaMacdext.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMacdext)
#endif

namespace hku {

TaMacdext::TaMacdext() : IndicatorImp("TA_MACDEXT", 3) {
    setParam<int>("fast_n", 12);
    setParam<int>("fast_matype", 0);
    setParam<int>("slow_n", 26);
    setParam<int>("slow_matype", 0);
    setParam<int>("signal_n", 9);
    setParam<int>("signal_matype", 0);
}

void TaMacdext::_checkParam(const string& name) const {
    if (name == "fast_n" || name == "slow_n") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 2 && n <= 100000, "{} must be in [2, 100000]!", name);
    } else if (name == "signal_n") {
        int signal_n = getParam<int>("signal_n");
        HKU_ASSERT(signal_n >= 1 && signal_n <= 100000);
    } else if (name == "fast_matype" || name == "slow_matype" || name == "signal_matype") {
        int matype = getParam<int>(name);
        HKU_CHECK(matype >= 0 && matype <= 8, "{} must be in [0, 8]!", name);
    }
}

void TaMacdext::_calculate(const Indicator& data) {
    int fast_n = getParam<int>("fast_n");
    TA_MAType fast_matype = (TA_MAType)getParam<int>("fast_matype");
    int slow_n = getParam<int>("slow_n");
    TA_MAType slow_matype = (TA_MAType)getParam<int>("slow_matype");
    int signal_n = getParam<int>("signal_n");
    TA_MAType signal_matype = (TA_MAType)getParam<int>("signal_matype");
    size_t total = data.size();
    int lookback =
      TA_MACDEXT_Lookback(fast_n, fast_matype, slow_n, slow_matype, signal_n, signal_matype);
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = data.discard() + lookback;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    const double* src = data.data();
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    auto* dst2 = this->data(2);

    int outBegIdx;
    int outNbElement;
    TA_MACDEXT(m_discard, total - 1, src, fast_n, fast_matype, slow_n, slow_matype, signal_n,
               signal_matype, &outBegIdx, &outNbElement, dst0 + m_discard, dst1 + m_discard,
               dst2 + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_MACDEXT(int fast_n, int slow_n, int signal_n, int fast_matype, int slow_matype,
                             int signal_matype) {
    auto p = make_shared<TaMacdext>();
    p->setParam<int>("fast_n", fast_n);
    p->setParam<int>("fast_matype", fast_matype);
    p->setParam<int>("slow_n", slow_n);
    p->setParam<int>("slow_matype", slow_matype);
    p->setParam<int>("signal_n", signal_n);
    p->setParam<int>("signal_matype", signal_matype);
    return Indicator(p);
}

} /* namespace hku */
