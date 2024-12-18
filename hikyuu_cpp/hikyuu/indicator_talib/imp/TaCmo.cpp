/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include "ta-lib/ta_func.h"
#include "TaCmo.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaCmo)
#endif

namespace hku {

TaCmo::TaCmo() : IndicatorImp("TA_CMO", 1) {
    setParam<int>("n", 14);
}

TaCmo::~TaCmo() {}

void TaCmo::_checkParam(const string &name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HKU_ASSERT(n >= 2 && n <= 100000);
    }
}

void TaCmo::_calculate(const Indicator &data) {
    int n = getParam<int>("n");
    m_discard = data.discard() + TA_CMO_Lookback(n);
    size_t total = data.size();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data();
    auto *dst = this->data();

    int outBegIdx;
    int outNbElement;
    TA_CMO(data.discard(), total - 1, src, n, &outBegIdx, &outNbElement, dst + m_discard);
}

Indicator HKU_API TA_CMO(int n) {
    auto p = make_shared<TaCmo>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
