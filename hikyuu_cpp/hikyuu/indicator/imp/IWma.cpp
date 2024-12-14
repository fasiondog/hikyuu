/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */

#include "IWma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IWma)
#endif

namespace hku {

IWma::IWma() : IndicatorImp("WMA", 1) {
    setParam<int>("n", 22);
}

IWma::~IWma() {}

void IWma::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_CHECK(getParam<int>("n") >= 1, "WMA n must >= 1!");
    }
}

void IWma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");
    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    if (n == 1) {
        memcpy(dst, src, total * sizeof(price_t));
        return;
    }

    size_t startIdx = m_discard;
    size_t outIdx = m_discard;
    size_t trailingIdx = startIdx - 1;

    value_t tempReal = 0.0, periodSum = 0.0, periodSub = 0.0;
    size_t inIdx = ind.discard();
    size_t i = 1;
    while (inIdx < startIdx) {
        tempReal = src[inIdx++];
        periodSub += tempReal;
        periodSum += tempReal * i;
        i++;
    }

    size_t endIdx = total - 1;
    value_t trailingValue = 0.;
    value_t divider = n * (n + 1) / 2.0;
    while (inIdx <= endIdx) {
        tempReal = src[inIdx++];
        periodSub += tempReal;
        periodSub -= trailingValue;
        periodSum += tempReal * n;
        // HKU_INFO("{}: {}, {} ", inIdx - 1, periodSum, divider);
        trailingValue = src[trailingIdx++];
        dst[outIdx++] = periodSum / divider;
        periodSum -= periodSub;
    }
}

void IWma::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    price_t sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += ind[i];
    }
    _set(sum / (curPos - start + 1), curPos);
}

Indicator HKU_API WMA(int n) {
    IndicatorImpPtr p = make_shared<IWma>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

// Indicator HKU_API MA(const IndParam& n) {
//     IndicatorImpPtr p = make_shared<IWma>();
//     p->setIndParam("n", n);
//     return Indicator(p);
// }

} /* namespace hku */
