/*
 * IKurtosis.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include <climits>
#include "IKurtosis.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IKurtosis)
#endif

namespace hku {

IKurtosis::IKurtosis() : IndicatorImp("KURT", 1) {
    setParam<int>("n", 10);
}

IKurtosis::~IKurtosis() {}

void IKurtosis::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>(name);
        HKU_ASSERT((n == 0 || n >= 4) && n < INT_MAX);
    }
}

void IKurtosis::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");

    auto const* src = data.data();
    auto* dst = this->data();

    // n == 0: expanding-window excess kurtosis over [data.discard(), i], skipping non-finite
    // points; moment denominators use the running finite count.
    if (0 == n) {
        m_discard = data.discard() + 3;
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
        size_t start_pos = data.discard();
        value_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
        value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;
        size_t valid = 0;
        value_t d0 = src[start_pos] - k;
        if (std::isfinite(d0)) {
            value_t d02 = d0 * d0;
            ex += d0;
            ex2 += d02;
            ex3 += d02 * d0;
            ex4 += d02 * d02;
            valid++;
        }
        for (size_t i = start_pos + 1; i < total; ++i) {
            value_t d = src[i] - k;
            if (std::isfinite(d)) {
                value_t d2 = d * d;
                ex += d;
                ex2 += d2;
                ex3 += d2 * d;
                ex4 += d2 * d2;
                valid++;
            }
            if (i < m_discard || valid == 0)
                continue;
            value_t nv = (value_t)valid;
            value_t var = std::max(0.0, ex2 / nv - ex * ex / (nv * nv));
            value_t std_dev_pow4 = var * var;
            if (var == 0) {
                dst[i] = -3.0;
            } else {
                value_t ex_over_n = ex / nv;
                value_t ex2_over_n = ex2 / nv;
                value_t fourth_moment = ex4 / nv - 4 * (ex3 / nv) * ex_over_n +
                                        6 * ex2_over_n * ex_over_n * ex_over_n -
                                        3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
                dst[i] = fourth_moment / std_dev_pow4 - 3.0;
            }
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
    value_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;
    size_t valid = 0;

    for (size_t i = start_pos; i < first_end; i++) {
        value_t d = src[i] - k;
        if (std::isfinite(d)) {
            value_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            ex4 += d2 * d2;
            valid++;
        }
    }

    auto emit = [&](size_t i) {
        if (valid == 0)
            return;
        value_t nv = (value_t)valid;
        value_t var = std::max(0.0, ex2 / nv - ex * ex / (nv * nv));
        if (var == 0) {
            dst[i] = -3.0;
            return;
        }
        value_t std_dev_pow4 = var * var;
        value_t ex_over_n = ex / nv;
        value_t ex2_over_n = ex2 / nv;
        value_t fourth_moment = ex4 / nv - 4 * (ex3 / nv) * ex_over_n +
                                6 * ex2_over_n * ex_over_n * ex_over_n -
                                3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
        dst[i] = fourth_moment / std_dev_pow4 - 3.0;
    };
    emit(first_end - 1);

    for (size_t i = first_end; i < total; i++) {
        value_t d_pre = src[i - n] - k;
        if (std::isfinite(d_pre)) {
            value_t dp2 = d_pre * d_pre;
            ex -= d_pre;
            ex2 -= dp2;
            ex3 -= dp2 * d_pre;
            ex4 -= dp2 * dp2;
            valid--;
        }
        value_t d = src[i] - k;
        if (std::isfinite(d)) {
            value_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            ex4 += d2 * d2;
            valid++;
        }
        emit(i);
    }
}

bool IKurtosis::supportIncrementCalculate() const {
    return getParam<int>("n") != 0;
}

size_t IKurtosis::min_increment_start() const {
    return getParam<int>("n");
}

void IKurtosis::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();

    value_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;
    size_t valid = 0;
    for (size_t i = start_pos - n; i < start_pos; i++) {
        value_t d = src[i] - k;
        if (std::isfinite(d)) {
            value_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            ex4 += d2 * d2;
            valid++;
        }
    }

    for (size_t i = start_pos; i < total; i++) {
        value_t d_pre = src[i - n] - k;
        if (std::isfinite(d_pre)) {
            value_t dp2 = d_pre * d_pre;
            ex -= d_pre;
            ex2 -= dp2;
            ex3 -= dp2 * d_pre;
            ex4 -= dp2 * dp2;
            valid--;
        }
        value_t d = src[i] - k;
        if (std::isfinite(d)) {
            value_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            ex4 += d2 * d2;
            valid++;
        }
        if (valid == 0)
            continue;
        value_t nv = (value_t)valid;
        value_t var = std::max(0.0, ex2 / nv - ex * ex / (nv * nv));
        if (var == 0) {
            dst[i] = -3.0;
            continue;
        }
        value_t std_dev_pow4 = var * var;
        value_t ex_over_n = ex / nv;
        value_t ex2_over_n = ex2 / nv;
        value_t fourth_moment = ex4 / nv - 4 * (ex3 / nv) * ex_over_n +
                                6 * ex2_over_n * ex_over_n * ex_over_n -
                                3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
        dst[i] = fourth_moment / std_dev_pow4 - 3.0;
    }
}

void IKurtosis::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step < 4, void());

    size_t start = _get_step_start(curPos, step, ind.discard());
    HKU_IF_RETURN(start != curPos + 1 - step, void());

    value_t k = std::isfinite(ind[start]) ? ind[start] : 0.0;
    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;
    size_t valid = 0;
    for (size_t i = start; i <= curPos; i++) {
        value_t d = ind[i] - k;
        if (std::isfinite(d)) {
            value_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            ex4 += d2 * d2;
            valid++;
        }
    }
    if (valid == 0) {
        _set(Null<value_t>(), curPos);
        return;
    }
    value_t nv = (value_t)valid;
    value_t var = std::max(0.0, ex2 / nv - ex * ex / (nv * nv));
    if (var == 0) {
        _set(-3.0, curPos);
        return;
    }
    value_t std_dev_pow4 = var * var;
    value_t ex_over_n = ex / nv;
    value_t ex2_over_n = ex2 / nv;
    value_t fourth_moment = ex4 / nv - 4 * (ex3 / nv) * ex_over_n +
                            6 * ex2_over_n * ex_over_n * ex_over_n -
                            3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
    _set(fourth_moment / std_dev_pow4 - 3.0, curPos);
}

Indicator HKU_API KURT(int n) {
    IndicatorImpPtr p = make_shared<IKurtosis>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API KURT(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IKurtosis>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
