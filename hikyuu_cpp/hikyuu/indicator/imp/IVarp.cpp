/*
 * IVarp.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include <climits>
#include "IVarp.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IVarp)
#endif

namespace hku {

IVarp::IVarp() : IndicatorImp("VARP", 1) {
    setParam<int>("n", 10);
}

IVarp::~IVarp() {}

void IVarp::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>(name);
        HKU_ASSERT((n == 0 || n >= 2) && n < INT_MAX);
    }
}

void IVarp::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");

    auto const* src = data.data();
    auto* dst = this->data();

    // n == 0: expanding-window population variance over [data.discard(), i], skipping non-finite
    // points; population size is the running count of finite points, not the raw index distance.
    if (0 == n) {
        m_discard = data.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
        size_t start_pos = data.discard();
        price_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
        price_t ex = 0.0, ex2 = 0.0;
        size_t valid = 0;
        price_t d0 = src[start_pos] - k;
        if (std::isfinite(d0)) {
            ex += d0;
            ex2 += d0 * d0;
            valid++;
        }
        dst[start_pos] = valid == 0 ? Null<value_t>() : 0.0;
        for (size_t i = start_pos + 1; i < total; ++i) {
            price_t d = src[i] - k;
            if (std::isfinite(d)) {
                ex += d;
                ex2 += d * d;
                valid++;
            }
            if (valid == 0)
                continue;
            price_t nv = (price_t)valid;
            dst[i] = std::max<price_t>(0.0, ex2 / nv - ex * ex / (nv * nv));
        }
        return;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

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
    }

    auto emit = [&](size_t i) {
        if (valid == 0)
            return;
        price_t nv = (price_t)valid;
        dst[i] = std::max<price_t>(0.0, ex2 / nv - ex * ex / (nv * nv));
    };
    emit(first_end - 1);

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
        emit(i);
    }
}

bool IVarp::supportIncrementCalculate() const {
    return getParam<int>("n") != 0;
}

size_t IVarp::min_increment_start() const {
    return getParam<int>("n");
}

void IVarp::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();

    price_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
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
        dst[i] = std::max<price_t>(0.0, ex2 / nv - ex * ex / (nv * nv));
    }
}

void IVarp::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step < 2, void());

    size_t start = _get_step_start(curPos, step, ind.discard());
    HKU_IF_RETURN(start != curPos + 1 - step, void());

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
    _set(std::max<price_t>(0.0, (ex2 - ex * ex / nv) / nv), curPos);
}

Indicator HKU_API VARP(int n) {
    IndicatorImpPtr p = make_shared<IVarp>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API VARP(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IVarp>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
