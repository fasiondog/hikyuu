/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaStochrsi.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaStochrsi)
#endif

namespace hku {

TaStochrsi::TaStochrsi() : IndicatorImp("TA_STOCHRSI", 2) {
    setParam<int>("n", 14);
    setParam<int>("fastk_n", 5);
    setParam<int>("fastd_n", 3);
    setParam<int>("matype", 0);
}

void TaStochrsi::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 2 && n <= 100000, "n must in [2, 100000]");
    } else if (name == "fastk_n" || name == "fastd_n") {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 1 && n <= 100000, "{} must in [1, 100000]", name);
    } else if (name == "matype") {
        int matype = getParam<int>("matype");
        HKU_ASSERT(matype >= 0 && matype <= 8);
    }
}

void TaStochrsi::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    int fastk_n = getParam<int>("fastk_n");
    int fastd_n = getParam<int>("fastd_n");
    TA_MAType matype = (TA_MAType)getParam<int>("matype");
    size_t total = data.size();
    int lookback = TA_STOCHRSI_Lookback(n, fastk_n, fastd_n, matype);
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

    int outBegIdx;
    int outNbElement;
    TA_STOCHRSI(m_discard, total - 1, src, n, fastk_n, fastd_n, matype, &outBegIdx, &outNbElement,
                dst0 + m_discard, dst1 + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_STOCHRSI(int n, int fastk_n, int fastd_n, int matype) {
    auto p = make_shared<TaStochrsi>();
    p->setParam<int>("n", n);
    p->setParam<int>("fastk_n", fastk_n);
    p->setParam<int>("fastd_n", fastd_n);
    p->setParam<int>("matype", matype);
    return Indicator(p);
}

} /* namespace hku */
