/*
 * IDevsq.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#include "IDevsq.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDevsq)
#endif

namespace hku {

IDevsq::IDevsq() : IndicatorImp("DEVSQ", 1) {
    setParam<int>("n", 10);
}

IDevsq::~IDevsq() {}

void IDevsq::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 2);
    }
}

void IDevsq::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

size_t IDevsq::min_increment_start() const {
    return getParam<int>("n");
}

void IDevsq::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    int n = getParam<int>("n");

    auto const* src = data.data();
    auto* dst = this->data();

    // Two passes per window: the mean, then the squared deviations; NaN inside a window propagates
    // to the output (same semantics as the dynamic path). A rolling mean update cannot do that:
    // when a NaN left the window the update was skipped and the value-initialized 0.0 was silently
    // used as the mean, so the next fully valid window emitted the sum of squares instead of the
    // sum of squared deviations (the final loop is already O(n) per bar, so this stays O(n) per
    // bar)
    for (size_t i = start_pos; i < total; ++i) {
        price_t sum = 0.0;
        for (size_t j = i + 1 - n; j <= i; ++j) {
            sum += src[j];
        }
        price_t mean = sum / n;

        sum = 0.0;
        for (size_t j = i + 1 - n; j <= i; ++j) {
            sum += std::pow(src[j] - mean, 2);
        }
        dst[i] = sum;
    }
}

void IDevsq::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos + 1 < ind.discard() + step) {
        return;
    }
    price_t sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += ind[i];
    }
    price_t mean = sum / (curPos - start + 1);
    sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += std::pow(ind[i] - mean, 2);
    }
    _set(sum, curPos);
}

Indicator HKU_API DEVSQ(int n) {
    IndicatorImpPtr p = make_shared<IDevsq>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API DEVSQ(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IDevsq>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
