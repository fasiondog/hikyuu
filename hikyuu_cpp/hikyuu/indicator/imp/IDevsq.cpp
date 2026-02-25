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

    std::vector<price_t> ma(total);
    size_t start = start_pos + 1 - n;
    price_t sum = 0.0;
    for (size_t i = start; i <= start_pos; ++i) {
        if (!std::isnan(src[i])) {
            sum += src[i];
        }
    }

    ma[start_pos] = sum / n;
    for (size_t i = start_pos + 1; i < total; ++i) {
        if (!std::isnan(src[i]) && !std::isnan(src[i - n])) {
            sum = src[i] + sum - src[i - n];
            ma[i] = sum / n;
        }
    }

    auto const* mean = ma.data();
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
        sum = 0.0;
        size_t start = i + 1 - n;
        for (size_t j = start; j <= i; ++j) {
            sum += std::pow(src[j] - mean[i], 2);
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
