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

    // n == 0: expanding-window population skewness over [data.discard(), i], skipping non-finite
    // points; the moment denominators use the running finite count.
    if (0 == n) {
        m_discard = data.discard() + 2;
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
        size_t start_pos = data.discard();
        price_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
        price_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
        size_t valid = 0;
        price_t d0 = src[start_pos] - k;
        if (std::isfinite(d0)) {
            ex += d0;
            ex2 += d0 * d0;
            ex3 += d0 * d0 * d0;
            valid++;
        }
        for (size_t i = start_pos + 1; i < total; ++i) {
            price_t d = src[i] - k;
            if (std::isfinite(d)) {
                price_t d2 = d * d;
                ex += d;
                ex2 += d2;
                ex3 += d2 * d;
                valid++;
            }
            if (i < m_discard || valid == 0)
                continue;
            price_t nv = (price_t)valid;
            price_t var = std::max<price_t>(0.0, ex2 / nv - ex * ex / (nv * nv));
            price_t std_dev = std::sqrt(var);
            if (std_dev == 0) {
                dst[i] = 0.0;
            } else {
                price_t ex_over_n = ex / nv;
                price_t third_moment =
                  ex3 / nv - 3 * (ex2 / nv) * ex_over_n + 2 * ex_over_n * ex_over_n * ex_over_n;
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

    size_t start_pos = data.discard();
    size_t first_end = start_pos + n >= total ? total : start_pos + n;
    price_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
    price_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
    size_t valid = 0;

    for (size_t i = start_pos; i < first_end; i++) {
        price_t d = src[i] - k;
        if (std::isfinite(d)) {
            price_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            valid++;
        }
    }

    auto emit = [&](size_t i) {
        if (valid == 0)
            return;
        price_t nv = (price_t)valid;
        price_t var = std::max<price_t>(0.0, ex2 / nv - ex * ex / (nv * nv));
        price_t std_dev = std::sqrt(var);
        if (std_dev == 0) {
            dst[i] = 0.0;
            return;
        }
        price_t ex_over_n = ex / nv;
        price_t third_moment =
          ex3 / nv - 3 * (ex2 / nv) * ex_over_n + 2 * ex_over_n * ex_over_n * ex_over_n;
        dst[i] = third_moment / (std_dev * std_dev * std_dev);
    };
    emit(first_end - 1);

    for (size_t i = first_end; i < total; i++) {
        price_t d_pre = src[i - n] - k;
        if (std::isfinite(d_pre)) {
            price_t dp2 = d_pre * d_pre;
            ex -= d_pre;
            ex2 -= dp2;
            ex3 -= dp2 * d_pre;
            valid--;
        }
        price_t d = src[i] - k;
        if (std::isfinite(d)) {
            price_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            valid++;
        }
        emit(i);
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

    price_t k = std::isfinite(src[start_pos]) ? src[start_pos] : 0.0;
    price_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
    size_t valid = 0;
    for (size_t i = start_pos - n; i < start_pos; i++) {
        price_t d = src[i] - k;
        if (std::isfinite(d)) {
            price_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            valid++;
        }
    }

    for (size_t i = start_pos; i < total; i++) {
        price_t d_pre = src[i - n] - k;
        if (std::isfinite(d_pre)) {
            price_t dp2 = d_pre * d_pre;
            ex -= d_pre;
            ex2 -= dp2;
            ex3 -= dp2 * d_pre;
            valid--;
        }
        price_t d = src[i] - k;
        if (std::isfinite(d)) {
            price_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            valid++;
        }
        if (valid == 0)
            continue;
        price_t nv = (price_t)valid;
        price_t var = std::max<price_t>(0.0, ex2 / nv - ex * ex / (nv * nv));
        price_t std_dev = std::sqrt(var);
        if (std_dev == 0) {
            dst[i] = 0.0;
        } else {
            price_t ex_over_n = ex / nv;
            price_t third_moment =
              ex3 / nv - 3 * (ex2 / nv) * ex_over_n + 2 * ex_over_n * ex_over_n * ex_over_n;
            dst[i] = third_moment / (std_dev * std_dev * std_dev);
        }
    }
}

void ISkewness::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step < 3, void());

    size_t start = _get_step_start(curPos, step, ind.discard());
    HKU_IF_RETURN(start != curPos + 1 - step, void());

    price_t k = std::isfinite(ind[start]) ? ind[start] : 0.0;
    price_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
    size_t valid = 0;
    for (size_t i = start; i <= curPos; i++) {
        price_t d = ind[i] - k;
        if (std::isfinite(d)) {
            price_t d2 = d * d;
            ex += d;
            ex2 += d2;
            ex3 += d2 * d;
            valid++;
        }
    }
    if (valid == 0) {
        _set(Null<value_t>(), curPos);
        return;
    }
    price_t nv = (price_t)valid;
    price_t var = std::max<price_t>(0.0, ex2 / nv - ex * ex / (nv * nv));
    price_t std_dev = std::sqrt(var);
    if (std_dev == 0) {
        _set(0.0, curPos);
        return;
    }
    price_t ex_over_n = ex / nv;
    price_t third_moment =
      ex3 / nv - 3 * (ex2 / nv) * ex_over_n + 2 * ex_over_n * ex_over_n * ex_over_n;
    _set(third_moment / (std_dev * std_dev * std_dev), curPos);
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
