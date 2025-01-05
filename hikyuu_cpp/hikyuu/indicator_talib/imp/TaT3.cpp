/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaT3.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaT3)
#endif

namespace hku {

TaT3::TaT3() : IndicatorImp("TA_T3", 1) {
    setParam<int>("n", 5);
    setParam<double>("vfactor", 0.7);
}

void TaT3::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 2 && n <= 100000, "n must >= 1 and <= 100000 ");
    } else if (name == "vfactor") {
        double vfactor = getParam<double>("vfactor");
        HKU_ASSERT(vfactor >= 0.0 && vfactor <= 1.0);
    }
}

void TaT3::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    double vfactor = getParam<double>("vfactor");
    size_t total = data.size();
    int lookback = TA_T3_Lookback(n, vfactor);
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
    auto* dst = this->data();
    int outBegIdx;
    int outNbElement;
    TA_T3(m_discard, total - 1, src, n, vfactor, &outBegIdx, &outNbElement, dst + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_T3(int n, double vfactor) {
    auto p = make_shared<TaT3>();
    p->setParam<int>("n", n);
    p->setParam<double>("vfactor", vfactor);
    return Indicator(p);
}

} /* namespace hku */
