/*
 * IVar.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "IVar.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IVar)
#endif

namespace hku {

IVar::IVar() : IndicatorImp("VAR", 1) {
    setParam<int>("n", 10);
}

IVar::~IVar() {}

void IVar::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>(name);
        HKU_ASSERT(n >= 2 || n == 0);
    }
}

void IVar::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    if (n == 0) {
        n = total;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = data.data();
    auto* dst = this->data();

    vector<price_t> pow_buf(data.size());

    size_t start_pos = data.discard();
    size_t first_end = start_pos + n >= total ? total : start_pos + n;
    value_t ex = 0.0, ex2 = 0.0;

    value_t k = src[start_pos];
    for (size_t i = start_pos; i < first_end; i++) {
        value_t d = src[i] - k;
        ex += d;
        value_t d_pow = d * d;
        pow_buf[i] = d_pow;
        ex2 += d_pow;
    }

    value_t p1 = 1. / (n - 1);
    value_t p2 = 1. / (n * (n - 1.));
    dst[first_end - 1] = ex2 * p1 - ex * ex * p2;

    for (size_t i = first_end, pre_ix = first_end - n; i < total; i++, pre_ix++) {
        ex -= src[pre_ix] - k;
        ex2 -= pow_buf[pre_ix];
        value_t d = src[i] - k;
        ex += d;
        value_t d_pow = d * d;
        pow_buf[i] = d_pow;
        ex2 += d_pow;
        dst[i] = ex2 * p1 - ex * ex * p2;
    }
}

void IVar::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step < 2, void());

    size_t start = _get_step_start(curPos, step, ind.discard());
    HKU_IF_RETURN(start != curPos + 1 - step, void());

    price_t ex = 0.0, ex2 = 0.0;
    price_t k = ind[start];
    for (size_t i = start; i <= curPos; i++) {
        price_t d = ind[i] - k;
        ex += d;
        ex2 += d * d;
    }
    _set((ex2 - ex * ex / step) / (step - 1), curPos);
}

Indicator HKU_API VAR(int n) {
    IndicatorImpPtr p = make_shared<IVar>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API VAR(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IVar>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
