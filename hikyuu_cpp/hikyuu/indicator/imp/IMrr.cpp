/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMrr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMrr)
#endif

namespace hku {

IMrr::IMrr() : IndicatorImp("MRR", 1) {
    setParam<int>("n", 0);
}

IMrr::~IMrr() {}

void IMrr::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IMrr::_calculate(const Indicator& ind) {
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
        value_t pre_min = src[m_discard];
        value_t max_rr = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (src[i] < pre_min || pre_min == 0.) {
                pre_min = src[i];
            }
            value_t rr = (src[i] <= pre_min) ? 0.0 : (src[i] / pre_min - 1.0);
            if (rr > max_rr) {
                max_rr = rr;
            }
            dst[i] = max_rr * 100.0;
        }
        return;
    }

    value_t pre_min = src[m_discard];
    value_t max_rr = 0.0;
    for (size_t i = m_discard; i < m_discard + n; ++i) {
        if (src[i] < pre_min || pre_min == 0.) {
            pre_min = src[i];
        }
        value_t rr = (src[i] <= pre_min) ? 0.0 : (src[i] / pre_min - 1.0);
        if (rr > max_rr) {
            max_rr = rr;
        }
        dst[i] = max_rr * 100.0;
    }

    if (m_discard + n < total) {
        _increment_calculate(ind, m_discard + n);
    }
}

bool IMrr::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t IMrr::min_increment_start() const {
    return getParam<int>("n");
}

void IMrr::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    // The standard maximum rise rate semantics: rr_j = src[j] / run_min_j - 1, where
    // run_min_j = min(src[window_left..j]) is the accumulated minimum up to j.
    // The original implementation wrongly used the global minimum of the window as the rise base
    // of all the points; when the lowest point of the window appears after the highest point it
    // introduces the look-ahead bias and overestimates the rise rate.
    // Here the original O(1) fast path state machine is abandoned (its judgment of
    // current_rr > window_max_rr does not hold in principle under the standard MRR semantics)
    // and it degenerates to an O(n) brute force scan per point, using the run_min base to
    // guarantee the correctness. It is symmetric to the IMdd fix.
    for (size_t i = start_pos; i < total; ++i) {
        Indicator::value_t current_price = src[i];
        if (std::isnan(current_price) || current_price <= 0.0) {
            // An invalid point does not write dst[i] and keeps the original value, the same as the
            // original semantics
            continue;
        }

        size_t window_left = i + 1 - n;
        Indicator::value_t run_min =
          0.0;  // Assigned at the first valid point, avoiding NaN pollution
        Indicator::value_t window_max_rr = 0.0;
        bool has_valid = false;
        for (size_t j = window_left; j <= i; ++j) {
            Indicator::value_t v = src[j];
            if (std::isnan(v) || v <= 0.0) {
                continue;
            }
            if (!has_valid) {
                run_min = v;  // Initialize run_min at the first valid point
                has_valid = true;
                continue;  // For the first point rr = v/v - 1 = 0, skip it
            }
            if (v < run_min) {
                run_min = v;
            }
            // run_min is always > 0 (the data is constrained to be positive), the division is safe
            Indicator::value_t rr = v / run_min - 1.0;
            if (rr > window_max_rr) {
                window_max_rr = rr;
            }
        }

        if (has_valid) {
            dst[i] = window_max_rr * 100.0;
        }
    }
}

Indicator HKU_API MRR(int n) {
    IndicatorImpPtr p = make_shared<IMrr>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku