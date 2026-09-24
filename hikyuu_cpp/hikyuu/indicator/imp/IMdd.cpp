/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMdd.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMdd)
#endif

namespace hku {

IMdd::IMdd() : IndicatorImp("MDD", 1) {
    setParam<int>("n", 0);
}

IMdd::~IMdd() {}

void IMdd::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IMdd::_calculate(const Indicator& ind) {
    m_discard = ind.discard();
    size_t total = ind.size();
    HKU_IF_RETURN(m_discard >= total, void());

    auto const* src = ind.data();
    auto* dst = this->data();

    size_t n = static_cast<size_t>(getParam<int>("n"));
    if (n == 0 || n > total - m_discard) {
        n = total - m_discard;
    }

    if (n == 1) {
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = 0.0;
        }
        return;
    }

    if (n == total - m_discard) {
        value_t pre_max = src[m_discard];
        value_t min_dd = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (src[i] > pre_max) {
                pre_max = src[i];
            }
            value_t dd = (src[i] >= pre_max || pre_max == 0.) ? 0.0 : (src[i] - pre_max) / pre_max;
            if (dd < min_dd) {
                min_dd = dd;
            }
            dst[i] = std::abs(min_dd * 100.0);
        }
        return;
    }

    value_t pre_max = src[m_discard];
    value_t min_dd = 0.0;
    for (size_t i = m_discard; i < m_discard + n; ++i) {
        if (src[i] > pre_max) {
            pre_max = src[i];
        }
        value_t dd = (src[i] >= pre_max || pre_max == 0.) ? 0.0 : (src[i] - pre_max) / pre_max;
        if (dd < min_dd) {
            min_dd = dd;
        }
        dst[i] = std::abs(min_dd * 100.0);
    }

    if (m_discard + n < total) {
        _increment_calculate(ind, m_discard + n);
    }
}

bool IMdd::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t IMdd::min_increment_start() const {
    return getParam<int>("n");
}

void IMdd::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    // The standard maximum drawdown semantics: dd_j = (run_max_j - src[j]) / run_max_j, where
    // run_max_j = max(src[window_left..j]) is the accumulated maximum up to j.
    // The original implementation wrongly used the global maximum of the window as the drawdown
    // base of all the points; when the highest point of the window appears after the lowest point
    // it introduces the look-ahead bias and overestimates the drawdown.
    // Here the original O(1) fast path state machine is abandoned (its judgment of
    // current_dd > window_max_dd does not hold in principle under the standard MDD semantics,
    // because the dd of different j uses a different run_max_j base), and it degenerates to an
    // O(n) brute force scan per point, using the run_max base to guarantee the correctness.
    for (size_t i = start_pos; i < total; ++i) {
        Indicator::value_t current_nav = src[i];
        if (std::isnan(current_nav) || current_nav <= 0.0) {
            // An invalid point does not write dst[i] and keeps the original value, the same as the
            // original semantics
            continue;
        }

        size_t window_left = i + 1 - n;
        Indicator::value_t run_max =
          0.0;  // Not initialized with src[window_left], avoiding NaN pollution
        Indicator::value_t window_max_dd = 0.0;
        bool has_valid = false;
        for (size_t j = window_left; j <= i; ++j) {
            Indicator::value_t v = src[j];
            if (std::isnan(v) || v <= 0.0) {
                continue;
            }
            has_valid = true;
            if (v > run_max) {
                run_max = v;
            }
            // run_max is always > 0 (the data is constrained to be positive), the division is safe
            Indicator::value_t dd = (run_max - v) / run_max;
            if (dd > window_max_dd) {
                window_max_dd = dd;
            }
        }

        if (has_valid) {
            dst[i] = window_max_dd * 100.0;
        }
    }
}

Indicator HKU_API MDD(int n) {
    IndicatorImpPtr p = make_shared<IMdd>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku