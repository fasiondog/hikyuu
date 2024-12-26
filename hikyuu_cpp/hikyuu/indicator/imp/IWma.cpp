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
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    if (n == 1) {
        memcpy(dst, src, total * sizeof(value_t));
        return;
    }

    value_t subsum = 0.0, sum = 0.0;
    for (size_t i = ind.discard(), end = m_discard + 1, count = 1; i < end; i++, count++) {
        subsum += src[i];
        sum += src[i] * count;
    }

    value_t divider = n * (n + 1) / 2.0;
    dst[m_discard] = sum / divider;

    size_t trailingIdx = ind.discard();
    for (size_t i = m_discard + 1; i < total; i++) {
        value_t tmp = src[i];
        sum -= subsum;
        subsum += tmp;
        subsum -= src[trailingIdx++];
        sum += tmp * n;
        dst[i] = sum / divider;
    }
}

void IWma::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (step < 1) {
        _set(Null<value_t>(), curPos);
        return;
    }

    if (step == 1) {
        _set(ind[curPos], curPos);
        return;
    }

    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos + 1 < step + start) {
        _set(Null<value_t>(), curPos);
        return;
    }

    value_t sum = 0.0;
    size_t n = 1;
    for (size_t i = start; i <= curPos; i++, n++) {
        sum += (ind[i] * n);
    }
    _set(sum / (step * (step + 1) / 2.), curPos);
}

Indicator HKU_API WMA(int n) {
    IndicatorImpPtr p = make_shared<IWma>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API WMA(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IWma>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
