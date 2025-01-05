/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaBbands.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaBbands)
#endif

namespace hku {

TaBbands::TaBbands() : IndicatorImp("TA_BBANDS", 3) {
    setParam<int>("n", 5);
    setParam<double>("nbdevup", 2.0);
    setParam<double>("nbdevdn", 2.0);
    setParam<int>("matype", 0);
}

void TaBbands::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 100000);
    } else if (name == "matype") {
        int matype = getParam<int>("matype");
        HKU_ASSERT(matype >= 0 && matype <= 8);
    } else if (name == "nbdevup") {
        HKU_ASSERT(!std::isnan(getParam<double>("nbdevup")));
    } else if (name == "nbdevdn") {
        HKU_ASSERT(!std::isnan(getParam<double>("nbdevdn")));
    }
}

void TaBbands::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    double nbdevup = getParam<double>("nbdevup");
    double nbdevdn = getParam<double>("nbdevdn");
    TA_MAType matype = (TA_MAType)getParam<int>("matype");
    size_t total = data.size();
    int lookback = TA_BBANDS_Lookback(n, nbdevup, nbdevdn, matype);
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = data.discard() + lookback;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    auto* dst2 = this->data(2);

    const double* src = data.data();

    int outBegIdx;
    int outNbElement;
    TA_BBANDS(m_discard, total - 1, src, n, nbdevup, nbdevdn, matype, &outBegIdx, &outNbElement,
              dst0 + m_discard, dst1 + m_discard, dst2 + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_BBANDS(int n, double nbdevup, double nbdevdn, int matype) {
    auto p = make_shared<TaBbands>();
    p->setParam<int>("n", n);
    p->setParam<double>("nbdevup", nbdevup);
    p->setParam<double>("nbdevdn", nbdevdn);
    p->setParam<int>("matype", matype);
    return Indicator(p);
}

} /* namespace hku */
