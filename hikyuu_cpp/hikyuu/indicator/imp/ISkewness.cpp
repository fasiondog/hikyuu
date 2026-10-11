/*
 * ISkewness.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include <climits>
#include "ISkewness.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISkewness)
#endif

namespace hku {

ISkewness::ISkewness() : IndicatorImp("SKEW", 1) {
    setParam<int>("n", 10);
}

ISkewness::~ISkewness() {}

void ISkewness::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>(name);
        HKU_ASSERT((n == 0 || n >= 3) && n < INT_MAX);
    }
}

void ISkewness::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");

    auto const* src = data.data();
    auto* dst = this->data();

    // n == 0: expanding-window population skewness, dst[i] uses [data.discard(), i]; num >= 3
    if (0 == n) {
        m_discard = data.discard() + 2;
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
        size_t start_pos = data.discard();
        value_t k = src[start_pos];
        value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
        for (size_t i = start_pos + 1; i < total; ++i) {
            value_t d = src[i] - k;
            ex += d;
            value_t d2 = d * d;
            ex2 += d2;
            ex3 += d2 * d;
            if (i < m_discard) {
                continue;
            }
            value_t num = (value_t)(i - start_pos + 1);
            value_t var = std::max(0.0, ex2 / num - ex * ex / (num * num));
            value_t std_dev = std::sqrt(var);
            if (std_dev == 0) {
                dst[i] = 0.0;
            } else {
                value_t ex_over_n = ex / num;
                value_t third_moment =
                  ex3 / num - 3 * (ex2 / num) * ex_over_n + 2 * ex_over_n * ex_over_n * ex_over_n;
                dst[i] = third_moment / (std_dev * std_dev * std_dev);
            }
        }
        return;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    // Ring of size n: at each rolling step the outgoing slot (i - n) and the incoming
    // slot (i) share the same index i % n, so a single cursor drives both read and write.
    size_t start_pos = data.discard();
    size_t first_end = start_pos + n >= total ? total : start_pos + n;
    vector<price_t> pow2_ring(n);
    vector<price_t> pow3_ring(n);

    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
    size_t ring_ix = start_pos % n;

    value_t k = src[start_pos];
    for (size_t i = start_pos; i < first_end; i++) {
        value_t d = src[i] - k;
        ex += d;
        value_t d2 = d * d;
        pow2_ring[ring_ix] = d2;
        ex2 += d2;
        value_t d3 = d2 * d;
        pow3_ring[ring_ix] = d3;
        ex3 += d3;
        if (++ring_ix == (size_t)n) {
            ring_ix = 0;
        }
    }

    value_t var = std::max(0.0, ex2 / n - ex * ex / ((value_t)n * n));
    value_t std_dev = sqrt(var);

    if (std_dev == 0) {
        dst[first_end - 1] = 0.0;
    } else {
        value_t ex_over_n = ex / n;
        value_t third_moment =
          ex3 / n - 3 * (ex2 / n) * ex_over_n + 2 * ex_over_n * ex_over_n * ex_over_n;
        value_t skewness = third_moment / (std_dev * std_dev * std_dev);
        dst[first_end - 1] = skewness;
    }

    for (size_t i = first_end; i < total; i++) {
        ex -= src[i - n] - k;
        ex2 -= pow2_ring[ring_ix];
        ex3 -= pow3_ring[ring_ix];

        value_t d = src[i] - k;
        ex += d;
        value_t d2 = d * d;
        pow2_ring[ring_ix] = d2;
        ex2 += d2;
        value_t d3 = d2 * d;
        pow3_ring[ring_ix] = d3;
        ex3 += d3;

        var = std::max(0.0, ex2 / n - ex * ex / ((value_t)n * n));
        std_dev = sqrt(var);

        if (std_dev == 0) {
            dst[i] = 0.0;
        } else {
            value_t ex_over_n = ex / n;
            value_t third_moment =
              ex3 / n - 3 * (ex2 / n) * ex_over_n + 2 * ex_over_n * ex_over_n * ex_over_n;
            value_t skewness = third_moment / (std_dev * std_dev * std_dev);
            dst[i] = skewness;
        }
        if (++ring_ix == (size_t)n) {
            ring_ix = 0;
        }
    }
}

bool ISkewness::supportIncrementCalculate() const {
    return getParam<int>("n") != 0;
}

size_t ISkewness::min_increment_start() const {
    return getParam<int>("n");
}

void ISkewness::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();

    vector<price_t> pow2_ring(n);
    vector<price_t> pow3_ring(n);

    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
    value_t k = src[start_pos];
    size_t ring_ix = start_pos % n;
    for (size_t i = start_pos - n; i < start_pos; i++) {
        value_t d = src[i] - k;
        ex += d;
        value_t d2 = d * d;
        pow2_ring[ring_ix] = d2;
        ex2 += d2;
        value_t d3 = d2 * d;
        pow3_ring[ring_ix] = d3;
        ex3 += d3;
        if (++ring_ix == (size_t)n) {
            ring_ix = 0;
        }
    }

    for (size_t i = start_pos; i < total; i++) {
        ex -= src[i - n] - k;
        ex2 -= pow2_ring[ring_ix];
        ex3 -= pow3_ring[ring_ix];

        value_t d = src[i] - k;
        ex += d;
        value_t d2 = d * d;
        pow2_ring[ring_ix] = d2;
        ex2 += d2;
        value_t d3 = d2 * d;
        pow3_ring[ring_ix] = d3;
        ex3 += d3;

        value_t var = std::max(0.0, ex2 / n - ex * ex / ((value_t)n * n));
        value_t std_dev = sqrt(var);

        if (std_dev == 0) {
            dst[i] = 0.0;
        } else {
            value_t ex_over_n = ex / n;
            value_t third_moment =
              ex3 / n - 3 * (ex2 / n) * ex_over_n + 2 * ex_over_n * ex_over_n * ex_over_n;
            value_t skewness = third_moment / (std_dev * std_dev * std_dev);
            dst[i] = skewness;
        }
        if (++ring_ix == (size_t)n) {
            ring_ix = 0;
        }
    }
}

void ISkewness::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step < 3, void());

    size_t start = _get_step_start(curPos, step, ind.discard());
    HKU_IF_RETURN(start != curPos + 1 - step, void());

    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
    value_t k = ind[start];
    for (size_t i = start; i <= curPos; i++) {
        value_t d = ind[i] - k;
        ex += d;
        value_t d2 = d * d;
        ex2 += d2;
        ex3 += d2 * d;
    }

    value_t var = std::max(0.0, ex2 / step - ex * ex / ((value_t)step * step));
    value_t std_dev = sqrt(var);

    if (std_dev == 0) {
        _set(0.0, curPos);
    } else {
        value_t ex_over_step = ex / step;
        value_t third_moment = ex3 / step - 3 * (ex2 / step) * ex_over_step +
                               2 * ex_over_step * ex_over_step * ex_over_step;
        value_t skewness = third_moment / (std_dev * std_dev * std_dev);
        _set(skewness, curPos);
    }
}

Indicator HKU_API SKEW(int n) {
    IndicatorImpPtr p = make_shared<ISkewness>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API SKEW(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ISkewness>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */