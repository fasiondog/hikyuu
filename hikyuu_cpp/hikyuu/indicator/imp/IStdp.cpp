/*
 * IStdp.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#include <climits>
#include "IStdp.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IStdp)
#endif

namespace hku {

IStdp::IStdp() : IndicatorImp("STDP", 1) {
    setParam<int>("n", 10);
}

IStdp::~IStdp() {}

void IStdp::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n == 0 || (n >= 2 && n < INT_MAX));
    }
}

void IStdp::_calculate(const Indicator& data) {
    size_t total = data.size();

    int n = getParam<int>("n");
    // n of 0 asks for the whole series, which has no warm up window to drop
    m_discard = data.discard() + (0 == n ? 0 : n - 1);
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    if (0 == n) {
        n = total;
    }

    auto const* src = data.data();
    auto* dst = this->data();

    size_t start_pos = data.discard();
    size_t first_end = start_pos + n >= total ? total : start_pos + n;
    price_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
    price_t ex = 0.0, ex2 = 0.0;
    size_t valid = 0;

    for (size_t i = start_pos; i < first_end; i++) {
        price_t d = src[i] - k;
        if (std::isfinite(d)) {
            ex += d;
            ex2 += d * d;
            valid++;
        }
        if (i < m_discard || valid == 0)
            continue;
        price_t nv = (price_t)valid;
        dst[i] = std::sqrt(std::max(0.0, (ex2 - ex * ex / nv) / nv));
    }

    for (size_t i = first_end; i < total; i++) {
        price_t d_pre = src[i - n] - k;
        if (std::isfinite(d_pre)) {
            ex -= d_pre;
            ex2 -= d_pre * d_pre;
            valid--;
        }
        price_t d = src[i] - k;
        if (std::isfinite(d)) {
            ex += d;
            ex2 += d * d;
            valid++;
        }
        if (valid == 0)
            continue;
        price_t nv = (price_t)valid;
        dst[i] = std::sqrt(std::max(0.0, (ex2 - ex * ex / nv) / nv));
    }
}

bool IStdp::supportIncrementCalculate() const {
    return getParam<int>("n") != 0;
}

size_t IStdp::min_increment_start() const {
    return getParam<int>("n");
}

void IStdp::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();

    price_t k = std::isfinite(src[start_pos - n]) ? src[start_pos - n] : 0.0;
    price_t ex = 0.0, ex2 = 0.0;
    size_t valid = 0;
    for (size_t i = start_pos - n; i < start_pos; i++) {
        price_t d = src[i] - k;
        if (std::isfinite(d)) {
            ex += d;
            ex2 += d * d;
            valid++;
        }
    }

    for (size_t i = start_pos; i < total; i++) {
        price_t d_pre = src[i - n] - k;
        if (std::isfinite(d_pre)) {
            ex -= d_pre;
            ex2 -= d_pre * d_pre;
            valid--;
        }
        price_t d = src[i] - k;
        if (std::isfinite(d)) {
            ex += d;
            ex2 += d * d;
            valid++;
        }
        if (valid == 0)
            continue;
        price_t nv = (price_t)valid;
        dst[i] = std::sqrt(std::max(0.0, (ex2 - ex * ex / nv) / nv));
    }
}

void IStdp::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step < 2, void());
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos - start + 1 < step) {
        _set(Null<price_t>(), curPos);
        return;
    }
    price_t k = std::isfinite(ind[start]) ? ind[start] : 0.0;
    price_t ex = 0.0, ex2 = 0.0;
    size_t valid = 0;
    for (size_t i = start; i <= curPos; i++) {
        price_t d = ind[i] - k;
        if (std::isfinite(d)) {
            ex += d;
            ex2 += d * d;
            valid++;
        }
    }
    if (valid == 0) {
        _set(Null<price_t>(), curPos);
        return;
    }
    price_t nv = (price_t)valid;
    _set(std::sqrt(std::max(0.0, (ex2 - ex * ex / nv) / nv)), curPos);
}

Indicator HKU_API STDP(int n) {
    IndicatorImpPtr p = make_shared<IStdp>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API STDP(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IStdp>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
